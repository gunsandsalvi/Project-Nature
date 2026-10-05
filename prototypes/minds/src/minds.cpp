// P6's minds (A3.3, A11, MND-09, MND-14, TIM-17): see minds.hpp. Pre-production code (research 00).
#include "minds.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <string_view>

#include "chance.hpp"
#include "draws.hpp"
#include "hash.hpp"
#include "maths.hpp"

namespace minds {

namespace {

using Clock = std::chrono::steady_clock;

double since(Clock::time_point t0) {
    return std::chrono::duration<double>(Clock::now() - t0).count();
}

// An array's element by an int index.
template <typename A>
auto& at(A& a, int i) {
    return a[static_cast<std::size_t>(i)];
}

constexpr int kChunk = 16;                                        // people in one piece of a window's choosing
constexpr int kBuckets = static_cast<int>((2 * kDay) / kWindow);  // the queue holds two days of windows
constexpr int kSquare = 8;                                        // cells a side of the company grid's squares, 32 m
constexpr int kGrid = kSide / kSquare;
constexpr float kSpeed = 1.0F;       // metres a second, walking
constexpr double kTalkReach = 20.0;  // metres (MND-09)
constexpr float kAdultAge = 14.0F;
constexpr std::int64_t kLongest = 20 * kHour;
constexpr double kTwoPi = 6.28318530717958623200e+00;

// The action people start the world doing.
constexpr int kResting = 10;
static_assert(std::string_view(kTable[kResting].name) == "rest where they are");

// How fast each need runs down in an hour awake and at rest, in points of 100 (BIO-09, MND-07).
constexpr std::array<float, kNeeds> kRunDown = {3.0F, 4.5F, 3.5F, 1.0F, 0.5F, 1.5F, 0.4F, 1.2F, 0.8F};

// A score's parts, which the reasons name: each need, then their nature, the hour and the cost.
constexpr int kParts = kNeeds + 3;
constexpr int kNature = kNeeds;
constexpr int kTime = kNeeds + 1;
constexpr int kCost = kNeeds + 2;
constexpr std::array<const char*, kParts> kPartWords = {"hungry",
                                                        "thirsty",
                                                        "tired",
                                                        "cold",
                                                        "uneasy",
                                                        "lonely",
                                                        "wants standing",
                                                        "curious",
                                                        "longs for closeness",
                                                        "their nature",
                                                        "the hour suits it",
                                                        "it costs little"};

int hour_of(std::int64_t t) {
    return static_cast<int>((t % kDay) / kHour);
}
int season_of(int day) {
    return (day % kYear) / 15;
}
bool by_day(int h) {
    return h >= 6 && h < 20;
}
bool at_night(int h) {
    return h < 6 || h >= 21;
}
bool in_evening(int h) {
    return h >= 17 && h < 23;
}
int bucket_of(std::int64_t t) {
    return static_cast<int>((t / kWindow) % kBuckets);
}
int square_of(int cell) {
    return ((cell_y(cell) / kSquare) * kGrid) + (cell_x(cell) / kSquare);
}
float clamp(float v, float lo, float hi) {
    return std::min(hi, std::max(lo, v));
}

bool habit_suits(Habit habit, int h) {
    switch (habit) {
        case Habit::kAny:
            return false;
        case Habit::kMorning:
            return h >= 6 && h < 10;
        case Habit::kMidday:
            return h >= 10 && h < 15;
        case Habit::kEvening:
            return h >= 17 && h < 22;
        case Habit::kNight:
            return h >= 21 || h < 6;
    }
    return false;
}

// How pressing a need is, by response curves (MND-09): the body's needs press hard as they run low, the mind's
// evenly, more so for those whose nature leans to them.
float press(int need, float level, const std::array<float, kTraits>& trait) {
    const float u = (100.0F - level) / 100.0F;
    switch (need) {
        case kHunger:
        case kThirst:
        case kRest:
        case kWarmth:
            return 2.0F * u * u;
        case kSafety:
            return u * (0.6F + (0.8F * trait[kNervous]));
        case kBelonging:
            return u * (0.6F + (0.8F * trait[kOutgoing]));
        case kStatus:
            return u * (0.6F + (0.8F * trait[kConscientious]));
        case kCuriosity:
            return u * (0.6F + (0.8F * trait[kOpenness]));
        default:
            return u * (0.6F + (0.8F * trait[kAgreeable]));
    }
}

// How fast a need runs down in an hour, by effort, sleep, night and season (BIO-09).
float run_down(int need, float effort, bool asleep, int hour, int season) {
    float r = at(kRunDown, need);
    switch (need) {
        case kHunger:
            r = (r + (2.0F * effort)) * (asleep ? 0.5F : 1.0F);
            break;
        case kThirst:
            r = (r + (3.0F * effort)) * (asleep ? 0.5F : 1.0F);
            break;
        case kRest:
            r = asleep ? 0.0F : r + (3.0F * effort);
            break;
        case kWarmth: {
            const float cold = (at_night(hour) ? 1.5F : 0.0F) + (season == 3 ? 2.0F : (season == 2 ? 0.8F : 0.0F));
            r = std::max(0.2F, r + cold - (asleep ? 1.5F : 0.0F));
            break;
        }
        default:
            break;
    }
    return r;
}

// Where a tie to someone is in a person's ties, kept in order of the other's ID, or where it would go.
int tie_at(const Person& p, std::int32_t other) {
    const auto* first = p.tie.data();
    const auto* found =
        std::lower_bound(first, first + p.ties, other, [](const Tie& t, std::int32_t o) { return t.other < o; });
    return static_cast<int>(found - first);
}

bool has_tie(const Person& p, int i, std::int32_t other) {
    return i < p.ties && at(p.tie, i).other == other;
}

// A tie found or made; nullptr when they already know as many people as a mind holds (MND-14).
Tie* tie_to(Person* p, std::int32_t other) {
    const int i = tie_at(*p, other);
    if (has_tie(*p, i, other)) {
        return &at(p->tie, i);
    }
    if (p->ties >= kTiesHeld) {
        return nullptr;
    }
    std::move_backward(p->tie.begin() + i, p->tie.begin() + p->ties, p->tie.begin() + p->ties + 1);
    at(p->tie, i) = {other, 0.0F, 0.0F};
    ++p->ties;
    return &at(p->tie, i);
}

}  // namespace

struct World::Scratch {
    explicit Scratch(int entrances) : paths(entrances) {}
    PathScratch paths;
    Trip trip;
    std::vector<std::int32_t> near;
    std::int64_t cut_short = 0;
    double choice = 0.0;
    double paths_time = 0.0;
    double talk = 0.0;
};

struct World::Option {
    std::int16_t action = -1;
    std::int32_t target = -1;
    std::int32_t cell = 0;
    float score = 0.0F;
    std::array<float, kParts> part{};
};

World::~World() = default;

World::World(const Settings& settings)
    : settings_(settings),
      land_(settings.seed),
      paths_(land_),
      pool_(settings.threads),
      queue_(static_cast<std::size_t>(kBuckets)),
      grid_start_(static_cast<std::size_t>((kGrid * kGrid) + 1), 0),
      grid_fill_(static_cast<std::size_t>(kGrid * kGrid), 0) {
    const int bands = std::min(settings_.bands, static_cast<int>(land_.camps.size()));
    settings_.bands = bands;
    const int count = bands * settings_.band_size;
    people_.resize(static_cast<std::size_t>(count));
    where_.resize(static_cast<std::size_t>(count));
    grid_.resize(static_cast<std::size_t>(count));
    for (int t = 0; t < pool_.threads(); ++t) {
        scratch_.push_back(std::make_unique<Scratch>(paths_.entrances()));
    }
    leader_.assign(static_cast<std::size_t>(bands), -1);
    neighbours_.resize(static_cast<std::size_t>(bands));
    const std::uint64_t seed = settings_.seed;
    for (int b = 0; b < bands; ++b) {
        const int camp = at(land_.camps, b).cell;
        std::vector<std::pair<double, int>> others;
        for (int o = 0; o < bands; ++o) {
            if (o != b) {
                others.emplace_back(metres(camp, at(land_.camps, o).cell), o);
            }
        }
        std::sort(others.begin(), others.end());
        for (std::size_t k = 0; k < 3; ++k) {
            at(neighbours_, b)[k] = others.empty() ? b : others[std::min(k, others.size() - 1)].second;
        }
        // what the band knows: the spots within 400 m of its camp, nearest first (MND-28)
        std::vector<std::int32_t> spots;
        land_.spots_near(camp, 400.0, &spots);
        std::sort(spots.begin(), spots.end(), [this, camp](std::int32_t a, std::int32_t c) {
            const double ma = metres(camp, at(land_.spots, a).cell);
            const double mc = metres(camp, at(land_.spots, c).cell);
            return ma < mc || (ma == mc && a < c);
        });
        for (int i = 0; i < settings_.band_size; ++i) {
            const int id = (b * settings_.band_size) + i;
            Person& p = at(people_, id);
            const auto key = static_cast<std::uint64_t>(id);
            p.band = b;
            const double a = samebits::chance(seed, key, 0, Draw::kBirth, 0);
            // three in five are adults, as in a band of foragers
            p.age = static_cast<float>(i * 5 < settings_.band_size * 3 ? 16.0 + (40.0 * a) : 2.0 + (11.0 * a));
            for (std::size_t t = 0; t < kTraits; ++t) {
                p.trait[t] = static_cast<float>(samebits::chance(seed, key, 0, Draw::kBirth, 1 + t));
            }
            for (std::size_t n = 0; n < kNeeds; ++n) {
                p.need[n] = static_cast<float>(55.0 + (40.0 * samebits::chance(seed, key, 0, Draw::kBirth, 10 + n)));
            }
            p.cell = camp;
            for (const std::int32_t s : spots) {
                if (p.places >= kPlacesHeld) {
                    break;
                }
                const Spot& spot = at(land_.spots, s);
                at(p.place, p.places++) = {spot.cell, s, -1, 0, spot.amount, spot.kind};
            }
            // everyone in the band, well known (CUL-30)
            for (int j = 0; j < settings_.band_size; ++j) {
                if (j != i) {
                    Tie* t = tie_to(&p, (b * settings_.band_size) + j);
                    const auto k = static_cast<std::uint64_t>(j);
                    t->familiarity = static_cast<float>(0.6 + (0.3 * samebits::chance(seed, key, 1, Draw::kBirth, k)));
                    t->opinion = static_cast<float>(-0.2 + (0.9 * samebits::chance(seed, key, 2, Draw::kBirth, k)));
                }
            }
            // and a few of the nearest band, barely
            const int other = at(neighbours_, b)[0];
            for (int j = 0; j < 4 && other != b; ++j) {
                Tie* t = tie_to(&p, (other * settings_.band_size) + ((i + (j * 7)) % settings_.band_size));
                t->familiarity = 0.15F;
                t->opinion = static_cast<float>(
                    0.4 * samebits::chance(seed, key, 3, Draw::kBirth, static_cast<std::uint64_t>(j)));
            }
            // the first activity: resting at camp until some time in the first hour
            const std::int64_t end =
                kWindow +
                (static_cast<std::int64_t>(std::floor(samebits::chance(seed, key, 0, Draw::kLength) * 55.0)) * kMinute);
            p.now.start = 0;
            p.now.end = end;
            p.now.full_end = end;
            p.now.from = camp;
            p.now.to = camp;
            p.now.action = static_cast<std::int16_t>(kResting);
            at(queue_, bucket_of(end)).push_back(id);
        }
    }
    choose_leaders();
}

void World::choose_leaders() {
    // each band's eldest adult leads it (CUL-30)
    for (int b = 0; b < static_cast<int>(leader_.size()); ++b) {
        std::int32_t best = -1;
        float age = -1.0F;
        for (int i = 0; i < settings_.band_size; ++i) {
            const int id = (b * settings_.band_size) + i;
            const Person& p = at(people_, id);
            if (p.age >= kAdultAge && p.age > age) {
                age = p.age;
                best = id;
            }
        }
        at(leader_, b) = best;
    }
}

void World::run_day() {
    const std::int64_t end = (static_cast<std::int64_t>(day_) + 1) * kDay;
    while (now_ < end) {
        window();
        now_ += kWindow;
        if (now_ % kHour == 0) {
            hourly();
        }
    }
    ++day_;
    daily();
}

// One window of five minutes: the activities ending in it land in order of their ends, then their people choose
// again in parallel, then each is queued by its new end.
void World::window() {
    auto& bucket = at(queue_, bucket_of(now_));
    if (bucket.empty()) {
        return;
    }
    auto t0 = Clock::now();
    std::sort(bucket.begin(), bucket.end(), [this](std::int32_t a, std::int32_t b) {
        const std::int64_t ea = at(people_, a).now.end;
        const std::int64_t eb = at(people_, b).now.end;
        return ea < eb || (ea == eb && a < b);
    });
    times_.other += since(t0);
    t0 = Clock::now();
    hear_time_ = 0.0;
    for (const std::int32_t id : bucket) {
        land_activity(id);
    }
    times_.land += since(t0) - hear_time_;
    times_.talk += hear_time_;
    t0 = Clock::now();
    snapshot();
    times_.other += since(t0);
    const int n = static_cast<int>(bucket.size());
    pool_.run((n + kChunk - 1) / kChunk, [this, &bucket, n](int piece, int thread) {
        Scratch* s = at(scratch_, thread).get();
        for (int i = piece * kChunk; i < std::min(n, (piece + 1) * kChunk); ++i) {
            decide(at(bucket, i), s);
        }
    });
    for (auto& s : scratch_) {
        paths_.merge(&s->paths);
        times_.choice += s->choice;
        times_.paths += s->paths_time;
        times_.talk += s->talk;
        s->choice = 0.0;
        s->paths_time = 0.0;
        s->talk = 0.0;
    }
    t0 = Clock::now();
    for (const std::int32_t id : bucket) {
        at(queue_, bucket_of(at(people_, id).now.end)).push_back(id);
    }
    bucket.clear();
    decisions_ += n;
    times_.other += since(t0);
}

std::int32_t World::cell_now(const Person& p, std::int64_t t) const {
    const Activity& a = p.now;
    const auto walked = static_cast<float>(t - a.start);
    if (a.walk <= 0.0F || walked >= a.walk) {
        return a.to;
    }
    if (walked <= 0.0F) {
        return a.from;
    }
    // along the straight line between, for company; the trip's own way is the view's to draw
    const float f = walked / a.walk;
    const int x = cell_x(a.from) + static_cast<int>(std::lround(static_cast<float>(cell_x(a.to) - cell_x(a.from)) * f));
    const int y = cell_y(a.from) + static_cast<int>(std::lround(static_cast<float>(cell_y(a.to) - cell_y(a.from)) * f));
    const int c = cell_at(x, y);
    return land_.passable(c) ? c : a.from;
}

void World::snapshot() {
    const int count = static_cast<int>(people_.size());
    std::fill(grid_start_.begin(), grid_start_.end(), 0);
    for (int i = 0; i < count; ++i) {
        const std::int32_t c = cell_now(at(people_, i), now_);
        at(where_, i) = c;
        ++at(grid_start_, square_of(c) + 1);
    }
    for (std::size_t g = 1; g < grid_start_.size(); ++g) {
        grid_start_[g] += grid_start_[g - 1];
    }
    std::copy(grid_start_.begin(), grid_start_.end() - 1, grid_fill_.begin());
    for (int i = 0; i < count; ++i) {
        at(grid_, at(grid_fill_, square_of(at(where_, i)))++) = i;
    }
}

void World::near(int cell, double within, std::int32_t self, std::vector<std::int32_t>* out) const {
    out->clear();
    const int gx = cell_x(cell) / kSquare;
    const int gy = cell_y(cell) / kSquare;
    for (int y = std::max(0, gy - 1); y <= std::min(kGrid - 1, gy + 1); ++y) {
        for (int x = std::max(0, gx - 1); x <= std::min(kGrid - 1, gx + 1); ++x) {
            const int g = (y * kGrid) + x;
            for (int k = at(grid_start_, g); k < at(grid_start_, g + 1); ++k) {
                const std::int32_t o = at(grid_, k);
                if (o != self && metres(cell, at(where_, o)) <= within) {
                    out->push_back(o);
                }
            }
        }
    }
}

void World::bring_needs(Person* p, std::int64_t to, float effort, bool asleep) const {
    if (to <= p->needs_at) {
        return;
    }
    const float hours = static_cast<float>(to - p->needs_at) / static_cast<float>(kHour);
    const int h = hour_of(to);
    const int season = season_of(day_);
    for (int n = 0; n < kNeeds; ++n) {
        at(p->need, n) = std::max(0.0F, at(p->need, n) - (run_down(n, effort, asleep, h, season) * hours));
    }
    p->needs_at = to;
}

void World::learn_place(Person* p, const Place& place) const {
    int oldest = 0;
    for (int i = 0; i < p->places; ++i) {
        Place& q = at(p->place, i);
        if (q.spot == place.spot) {
            if (place.day >= q.day) {
                q.amount = place.amount;
                q.day = place.day;
                q.told_by = place.told_by;
            }
            return;
        }
        if (q.day < at(p->place, oldest).day) {
            oldest = i;
        }
    }
    if (p->places < kPlacesHeld) {
        at(p->place, p->places++) = place;
    } else {
        at(p->place, oldest) = place;  // the place heard of longest ago gives way (MND-14)
    }
}

void World::see(Person* p, std::int32_t spot) const {
    const Spot& s = at(land_.spots, spot);
    learn_place(p, {s.cell, spot, -1, day_, s.amount, s.kind});
}

void World::remember(Person* p, std::int64_t tick, std::int32_t who, int what, float strength) const {
    at(p->memory, p->memory_next) = {tick, who, static_cast<std::int16_t>(what), strength};
    p->memory_next = (p->memory_next + 1) % kMemoriesHeld;
    p->memories = std::min(kMemoriesHeld, p->memories + 1);
}

// A talk heard (MND-33): the speaker's topics reach the listener, a place with who told it (MND-23), an opinion
// moving the listener's own by how much they trust the speaker, and news remembered; each knows the other better.
void World::hear(std::int32_t speaker, std::int32_t listener) {
    if (listener < 0 || listener == speaker) {
        return;
    }
    Person& a = at(people_, speaker);
    Person& b = at(people_, listener);
    const int i = tie_at(b, speaker);
    const float opinion = has_tie(b, i, speaker) ? at(b.tie, i).opinion : 0.0F;
    const float trust = 0.3F + (0.25F * (opinion + 1.0F));
    for (const Topic& topic : a.now.topics) {
        switch (topic.kind) {
            case 1: {
                Place place = at(a.place, topic.index);
                place.told_by = speaker;
                learn_place(&b, place);
                break;
            }
            case 2: {
                const Tie x = at(a.tie, topic.index);
                if (x.other != listener) {
                    Tie* y = tie_to(&b, x.other);
                    if (y != nullptr) {
                        y->opinion += 0.1F * trust * (x.opinion - y->opinion);
                        y->familiarity = std::max(y->familiarity, 0.05F);
                    }
                }
                break;
            }
            case 3: {
                const Memory m = at(a.memory, topic.index);
                remember(&b, a.now.end, speaker, m.what, 0.5F * m.strength);
                break;
            }
            default:
                break;
        }
    }
    for (const auto& [from, to] : {std::pair<Person*, std::int32_t>{&a, listener}, {&b, speaker}}) {
        Tie* t = tie_to(from, to);
        if (t != nullptr) {
            t->familiarity = std::min(1.0F, t->familiarity + 0.02F);
        }
    }
    b.need[kBelonging] = std::min(100.0F, b.need[kBelonging] + 5.0F);
}

// An activity's end (TIM-17): needs brought up to it, and its results landing, in full or the share it reached.
void World::land_activity(std::int32_t id) {
    Person& p = at(people_, id);
    const Activity a = p.now;
    const Action& act = at(kTable, a.action);
    bring_needs(&p, a.end, act.effort, act.result == Result::kSleep);
    p.cell = cell_now(p, a.end);
    const auto spent = static_cast<float>(a.end - a.start);
    if (spent < a.walk) {
        return;  // cut short on the way there: nothing done
    }
    const float work = static_cast<float>(a.full_end - a.start) - a.walk;
    const float share = work <= 0.0F ? 1.0F : clamp((spent - a.walk) / work, 0.0F, 1.0F);
    float yield = 1.0F;
    Camp& camp = at(land_.camps, p.band);
    Spot* spot = (act.target == Target::kSpot && a.target >= 0) ? &at(land_.spots, a.target) : nullptr;
    switch (act.result) {
        case Result::kEatCarried:
            yield = p.food >= 1.0F ? 1.0F : 0.0F;
            p.food -= yield;
            break;
        case Result::kEatStore:
            yield = camp.food >= 1.0F ? 1.0F : 0.0F;
            camp.food -= yield;
            break;
        case Result::kEatSpot:
            yield = spot->amount >= share ? 1.0F : 0.0F;
            spot->amount -= yield * share;
            see(&p, a.target);
            break;
        case Result::kDrinkCarried:
            yield = p.water >= 1.0F ? 1.0F : 0.0F;
            p.water -= yield;
            break;
        case Result::kGather: {
            const float take = std::min(spot->amount, 4.0F * share);
            spot->amount -= take;
            p.food += take;
            see(&p, a.target);
            break;
        }
        case Result::kHunt:
            if (spot->amount >= 1.0F &&
                samebits::chance(settings_.seed, static_cast<std::uint64_t>(id), static_cast<std::uint64_t>(a.end),
                                 Draw::kFind) < 0.35 * share) {
                spot->amount -= 1.0F;
                p.game += 1.0F;
            } else {
                yield = 0.3F;
            }
            see(&p, a.target);
            break;
        case Result::kFetchWater:
            p.water = 3.0F;
            break;
        case Result::kFetchWood: {
            const float take = std::min(spot->amount, 2.0F * share);
            spot->amount -= take;
            p.wood += take;
            see(&p, a.target);
            break;
        }
        case Result::kFetchFlint: {
            const float take = std::min(spot->amount, share);
            spot->amount -= take;
            p.flint += take;
            see(&p, a.target);
            break;
        }
        case Result::kStore:
            camp.food += p.food;
            camp.wood += p.wood;
            p.food = 0.0F;
            p.wood = 0.0F;
            break;
        case Result::kTendFire:
            yield = camp.wood >= 1.0F ? 1.0F : 0.0F;
            camp.wood -= yield;
            camp.fire += 3.0F * yield;
            break;
        case Result::kButcher:
            yield = p.game >= 1.0F ? 1.0F : 0.0F;
            p.game -= yield;
            camp.food += 12.0F * yield * share;
            break;
        case Result::kKnap:
            yield = p.flint >= 1.0F ? 1.0F : 0.0F;
            p.flint -= yield;
            break;
        case Result::kShareFood:
            yield = (p.food >= 1.0F && a.target >= 0) ? 1.0F : 0.0F;
            if (yield > 0.0F) {
                p.food -= 1.0F;
                Person& q = at(people_, a.target);
                q.need[kHunger] = std::min(100.0F, q.need[kHunger] + 30.0F);
            }
            break;
        case Result::kTalk: {
            const auto t0 = Clock::now();
            std::int32_t listener = -1;
            if (act.target == Target::kPerson) {
                listener = a.target;
            } else if (act.target == Target::kOtherCamp && a.target >= 0) {
                // someone of the band visited
                const auto k = static_cast<int>(samebits::chance(settings_.seed, static_cast<std::uint64_t>(id),
                                                                 static_cast<std::uint64_t>(a.end), Draw::kTalk, 1) *
                                                settings_.band_size);
                listener = (a.target * settings_.band_size) + k;
            }
            hear(id, listener);
            hear_time_ += since(t0);
            break;
        }
        case Result::kExplore:
            // what lies within 40 m of where they went (MND-28)
            land_.spots_near(a.to, 40.0, &found_);
            for (const std::int32_t s : found_) {
                see(&p, s);
            }
            break;
        case Result::kDrinkSpot:
        case Result::kSleep:
        case Result::kNone:
            break;
    }
    float felt = 0.0F;
    for (int n = 0; n < kNeeds; ++n) {
        const auto g = static_cast<float>(at(act.gain, n));
        if (g != 0.0F) {
            const float change = g * share * (g > 0.0F ? yield : 1.0F);
            at(p.need, n) = clamp(at(p.need, n) + change, 0.0F, 100.0F);
            felt += change;
        }
    }
    remember(&p, a.end, act.target == Target::kPerson ? a.target : -1, a.action, clamp(felt / 60.0F, -1.0F, 1.0F));
}

// A choice (MND-09): every action they could do now, scored by what it does for each need, weighted by how pressing
// the need is and by their nature, with the hour, effort, distance and risk; usually the best, sometimes one close
// behind; its trip found, and its end set, sooner if a need it doesn't meet will fall below 20 first (TIM-17).
// It writes only this person's activity.
void World::decide(std::int32_t id, Scratch* s) {
    const auto t0 = Clock::now();
    double path_time = 0.0;
    double talk_time = 0.0;
    Person& p = at(people_, id);
    const std::int64_t t = p.now.end;
    const int h = hour_of(t);
    const int season = season_of(day_);
    const Camp& camp = at(land_.camps, p.band);
    const bool adult = p.age >= kAdultAge;
    const std::uint64_t seed = settings_.seed;
    const auto key = static_cast<std::uint64_t>(id);
    const auto tick = static_cast<std::uint64_t>(t);

    // the nearest place of each kind they believe holds something and can reach: a place in another connected
    // region, such as ground ringed by rock, is passed over at once (MND-28, A11)
    const std::int32_t region = paths_.region(p.cell);
    std::array<int, kKinds> place{};
    place.fill(-1);
    std::array<double, kKinds> place_m{};
    place_m.fill(1.0e18);
    for (int i = 0; i < p.places; ++i) {
        const Place& pl = at(p.place, i);
        if (pl.amount < 0.5F || paths_.region(pl.cell) != region) {
            continue;
        }
        const double m = metres(p.cell, pl.cell);
        const auto k = static_cast<int>(pl.kind);
        if (m < at(place_m, k)) {
            at(place_m, k) = m;
            at(place, k) = i;
        }
    }

    // company within 20 m, where people stood at the window's start: the best known of them, and a child (MND-33)
    auto tt = Clock::now();
    near(p.cell, kTalkReach, id, &s->near);
    std::int32_t partner = -1;
    std::int32_t child = -1;
    float known = -1.0F;
    for (const std::int32_t o : s->near) {
        const int i = tie_at(p, o);
        const float f = has_tie(p, i, o) ? at(p.tie, i).familiarity : 0.0F;
        if (f > known) {
            known = f;
            partner = o;
        }
        if (child < 0 && at(people_, o).age < kAdultAge) {
            child = o;
        }
    }
    talk_time += since(tt);

    // every action they could do now, scored; the best four kept, in order of score, then of action
    std::array<Option, 4> top{};
    int kept = 0;
    Option o;
    for (int a = 0; a < kActions; ++a) {
        const Action& act = at(kTable, a);
        const unsigned g = act.gates;
        const bool barred = ((g & kByDay) != 0U && !by_day(h)) || ((g & kAtNight) != 0U && !at_night(h)) ||
                            ((g & kEvening) != 0U && !in_evening(h)) || ((g & kAdults) != 0U && !adult) ||
                            ((g & kChildren) != 0U && adult) || ((g & kFireLit) != 0U && camp.fire <= 0.0F) ||
                            ((g & kCompany) != 0U && s->near.empty()) || ((g & kHasFood) != 0U && p.food < 1.0F) ||
                            ((g & kHasWater) != 0U && p.water < 1.0F) || ((g & kHasWood) != 0U && p.wood < 1.0F) ||
                            ((g & kHasFlint) != 0U && p.flint < 1.0F) || ((g & kHasGame) != 0U && p.game < 1.0F) ||
                            ((g & kStoreFood) != 0U && camp.food < 1.0F) ||
                            ((g & kStoreWood) != 0U && camp.wood < 1.0F);
        if (barred) {
            continue;
        }
        float success = 1.0F;
        o.action = static_cast<std::int16_t>(a);
        o.target = -1;
        o.cell = p.cell;
        switch (act.target) {
            case Target::kSelf:
                break;
            case Target::kCamp:
                o.cell = camp.cell;
                o.target = p.band;
                break;
            case Target::kSpot: {
                const int i = at(place, static_cast<int>(act.spot));
                if (i < 0) {
                    continue;
                }
                const Place& pl = at(p.place, i);
                o.cell = pl.cell;
                o.target = pl.spot;
                success = act.spot == Kind::kWater ? 1.0F : std::min(1.0F, pl.amount / 2.0F);
                if (act.result == Result::kHunt) {
                    success *= 0.35F;
                }
                break;
            }
            case Target::kPerson: {
                const std::int32_t who = (g & kWithChild) != 0U ? child : partner;
                if (who < 0) {
                    continue;
                }
                o.target = who;
                o.cell = at(where_, who);
                break;
            }
            case Target::kLeader: {
                const std::int32_t l = at(leader_, p.band);
                if (l < 0 || l == id) {
                    continue;
                }
                o.target = l;
                o.cell = at(where_, l);
                break;
            }
            case Target::kFar:
            case Target::kNear: {
                const auto k = static_cast<std::uint64_t>(a);
                const double u = samebits::chance(seed, key, tick, Draw::kFar, k);
                const double v = samebits::chance(seed, key, tick, Draw::kFar, k + 100);
                const double r = act.target == Target::kFar ? 200.0 + (400.0 * v) : 20.0 + (40.0 * v);
                const int x =
                    cell_x(p.cell) + static_cast<int>(std::lround(samebits::cosine(kTwoPi * u) * r / kCellMetres));
                const int y =
                    cell_y(p.cell) + static_cast<int>(std::lround(samebits::sine(kTwoPi * u) * r / kCellMetres));
                if (x < 0 || y < 0 || x >= kSide || y >= kSide || !land_.passable(cell_at(x, y)) ||
                    paths_.region(cell_at(x, y)) != paths_.region(p.cell)) {
                    continue;
                }
                o.cell = cell_at(x, y);
                break;
            }
            case Target::kOtherCamp: {
                const auto k = static_cast<std::size_t>(samebits::chance(seed, key, tick, Draw::kFar, 999) * 3.0);
                const std::int32_t b = at(neighbours_, p.band)[std::min<std::size_t>(k, 2)];
                if (b == p.band) {
                    continue;
                }
                o.target = b;
                o.cell = at(land_.camps, b).cell;
                break;
            }
        }
        // paths run about a fifth longer than straight lines
        const double m = o.cell == p.cell ? 0.0 : metres(p.cell, o.cell) * 1.2;
        const auto walk_minutes = static_cast<float>(m / 60.0);
        const float hours = (walk_minutes + static_cast<float>(act.minutes)) / 60.0F;
        float score = 0.0F;
        for (int n = 0; n < kNeeds; ++n) {
            const float part = static_cast<float>(at(act.gain, n)) * press(n, at(p.need, n), p.trait) * success;
            at(o.part, n) = part;
            score += part;
        }
        at(o.part, kNature) = act.pull * (at(p.trait, static_cast<int>(act.trait)) - 0.5F) * 20.0F;
        at(o.part, kTime) = habit_suits(act.habit, h) ? 6.0F : 0.0F;
        at(o.part, kCost) =
            -(act.effort * hours * 8.0F) - (walk_minutes * 0.1F) - (act.risk * 40.0F * p.trait[kNervous]);
        score += at(o.part, kNature) + at(o.part, kTime) + at(o.part, kCost);
        o.score = score;
        int k = kept;
        while (k > 0 && at(top, k - 1).score < o.score) {
            --k;
        }
        if (k < 4) {
            for (int j = std::min(kept, 3); j > k; --j) {
                at(top, j) = at(top, j - 1);
            }
            at(top, k) = o;
            kept = std::min(4, kept + 1);
        }
    }

    // usually the best, sometimes one close behind, by keyed chance (TIM-16)
    int pick = 0;
    if (kept > 1 && top[0].score > 0.0F && top[1].score >= 0.9F * top[0].score &&
        samebits::chance(seed, key, tick, Draw::kPick) < 0.25) {
        pick = 1;
    }
    // the trip there; an option they cannot reach gives way to the next
    Trip& trip = s->trip;
    int chosen = -1;
    for (int k = 0; k < kept; ++k) {
        const int i = k == 0 ? pick : (k == pick ? 0 : k);
        trip.metres = 0.0F;
        trip.via.clear();
        if (at(top, i).cell == p.cell) {
            chosen = i;
            break;
        }
        const auto tp = Clock::now();
        const bool ok = paths_.trip(p.cell, at(top, i).cell, &s->paths, &trip);
        path_time += since(tp);
        if (ok) {
            chosen = i;
            break;
        }
    }
    Option rest;
    rest.action = static_cast<std::int16_t>(kResting);
    rest.cell = p.cell;
    const Option& c = chosen >= 0 ? at(top, chosen) : rest;
    const Action& act = at(kTable, c.action);

    Activity next;
    next.start = t;
    next.from = p.cell;
    next.to = c.cell;
    next.target = c.target;
    next.action = c.action;
    next.walk = trip.metres / kSpeed;
    const double u = samebits::chance(seed, key, tick, Draw::kLength);
    const double work =
        (static_cast<double>(act.minutes) + (static_cast<double>(act.spread) * ((2.0 * u) - 1.0))) * 60.0;
    auto length = static_cast<std::int64_t>(std::ceil(static_cast<double>(next.walk) + std::max(0.0, work)));
    length = std::min(kLongest, std::max(kWindow, length));
    next.end = t + length;
    next.full_end = next.end;
    // a need it doesn't meet falling below 20 ends it then (TIM-17)
    const bool asleep = act.result == Result::kSleep;
    for (int n = 0; n < kNeeds; ++n) {
        if (at(act.gain, n) > 0 || at(p.need, n) <= 20.0F) {
            continue;
        }
        const float r = run_down(n, act.effort, asleep, h, season);
        if (r <= 0.0F) {
            continue;
        }
        const auto cross = t + static_cast<std::int64_t>(((at(p.need, n) - 20.0F) / r) * static_cast<float>(kHour));
        if (cross < next.end) {
            next.end = std::max(t + kWindow, cross);
        }
    }
    s->cut_short += next.end < next.full_end ? 1 : 0;
    // the reasons (PRN-13): the three parts that most put it ahead of the best option it beat, and the two it beat
    const Option* beat = nullptr;
    const Option* beat2 = nullptr;
    for (int k = 0; k < kept; ++k) {
        if (k == chosen) {
            continue;
        }
        if (beat == nullptr) {
            beat = &at(top, k);
        } else if (beat2 == nullptr) {
            beat2 = &at(top, k);
        }
    }
    if (beat != nullptr) {
        std::array<float, kParts> lead{};
        for (int k = 0; k < kParts; ++k) {
            at(lead, k) = at(c.part, k) - at(beat->part, k);
        }
        for (std::size_t r = 0; r < 3; ++r) {
            int best = 0;
            for (int k = 1; k < kParts; ++k) {
                if (at(lead, k) > at(lead, best)) {
                    best = k;
                }
            }
            next.reasons[r] = static_cast<std::uint8_t>(best);
            at(lead, best) = -1.0e30F;
        }
        next.beaten[0] = beat->action;
        next.beaten[1] = beat2 != nullptr ? beat2->action : static_cast<std::int16_t>(-1);
    }
    // what they will pass on, if it is a talk (MND-33): from what they know now, heard when it ends
    if (act.result == Result::kTalk) {
        tt = Clock::now();
        int newest = -1;
        for (int i = 0; i < p.places; ++i) {
            const Place& pl = at(p.place, i);
            if (pl.amount >= 0.5F && (newest < 0 || pl.day > at(p.place, newest).day)) {
                newest = i;
            }
        }
        if (newest >= 0) {
            next.topics[0] = {1, newest};
        }
        if (p.ties > 0) {
            const auto i = static_cast<int>(samebits::chance(seed, key, tick, Draw::kTalk) * p.ties);
            next.topics[1] = {2, std::min(i, p.ties - 1)};
        }
        if (p.memories > 0) {
            next.topics[2] = {3, (p.memory_next + kMemoriesHeld - 1) % kMemoriesHeld};
        }
        talk_time += since(tt);
    }
    p.now = next;
    s->choice += since(t0) - path_time - talk_time;
    s->paths_time += path_time;
    s->talk += talk_time;
}

// Each game hour: fires burn down, and the slow changes settle (MND-14): mood from needs and recent memories,
// and every tie fading a little.
void World::hourly() {
    const auto t0 = Clock::now();
    for (Camp& c : land_.camps) {
        c.fire = std::max(0.0F, c.fire - 1.0F);
    }
    const int count = static_cast<int>(people_.size());
    pool_.run((count + 63) / 64, [this, count](int piece, int /*thread*/) {
        for (int i = piece * 64; i < std::min(count, (piece + 1) * 64); ++i) {
            Person& p = at(people_, i);
            float m = 0.0F;
            for (const float n : p.need) {
                m += (n - 50.0F) / 50.0F;
            }
            m /= static_cast<float>(kNeeds);
            float recent = 0.0F;
            for (int j = 1; j <= std::min(10, p.memories); ++j) {
                recent += at(p.memory, (p.memory_next + kMemoriesHeld - j) % kMemoriesHeld).strength;
            }
            p.mood = clamp(m + (0.05F * recent), -1.0F, 1.0F);
            for (int j = 0; j < p.ties; ++j) {
                at(p.tie, j).familiarity *= 0.9998F;
            }
        }
    });
    times_.other += since(t0);
}

void World::daily() {
    const auto t0 = Clock::now();
    land_.regrow(season_of(day_));
    for (Person& p : people_) {
        p.age += 1.0F / static_cast<float>(kYear);
    }
    choose_leaders();
    times_.other += since(t0);
}

std::int64_t World::cut_short() const {
    std::int64_t n = 0;
    for (const auto& s : scratch_) {
        n += s->cut_short;
    }
    return n;
}

TripCounts World::trips() const {
    TripCounts sum;
    for (const auto& s : scratch_) {
        sum.inside += s->paths.counts.inside;
        sum.cached += s->paths.counts.cached;
        sum.fresh += s->paths.counts.fresh;
        sum.none += s->paths.counts.none;
    }
    return sum;
}

std::uint64_t World::checksum() const {
    std::uint64_t h = samebits::kHashStart;
    for (const Person& p : people_) {
        for (const float n : p.need) {
            samebits::hash_into(&h, n);
        }
        for (const float v : {p.mood, p.food, p.water, p.wood, p.flint, p.game, p.now.walk}) {
            samebits::hash_into(&h, v);
        }
        for (const std::int64_t v :
             {p.needs_at, p.now.start, p.now.end, p.now.full_end, static_cast<std::int64_t>(p.cell),
              static_cast<std::int64_t>(p.now.to), static_cast<std::int64_t>(p.now.target),
              static_cast<std::int64_t>(p.now.action), static_cast<std::int64_t>(p.places),
              static_cast<std::int64_t>(p.ties), static_cast<std::int64_t>(p.memories)}) {
            samebits::hash_into(&h, static_cast<std::uint64_t>(v));
        }
        for (int i = 0; i < p.places; ++i) {
            samebits::hash_into(&h, at(p.place, i).amount);
            samebits::hash_into(&h, static_cast<std::uint64_t>(at(p.place, i).day));
        }
        for (int i = 0; i < p.ties; ++i) {
            samebits::hash_into(&h, static_cast<std::uint64_t>(at(p.tie, i).other));
            samebits::hash_into(&h, at(p.tie, i).opinion);
            samebits::hash_into(&h, at(p.tie, i).familiarity);
        }
    }
    for (const Spot& s : land_.spots) {
        samebits::hash_into(&h, s.amount);
    }
    for (const Camp& c : land_.camps) {
        for (const float v : {c.food, c.wood, c.fire}) {
            samebits::hash_into(&h, v);
        }
    }
    samebits::hash_into(&h, static_cast<std::uint64_t>(now_));
    return h;
}

std::string World::explain(int person) const {
    const Activity& a = at(people_, person).now;
    std::string out = at(kTable, a.action).name;
    std::string why;
    for (const std::uint8_t r : a.reasons) {
        if (r < kParts) {
            why += why.empty() ? "" : "; ";
            why += at(kPartWords, r);
        }
    }
    if (!why.empty()) {
        out += ", because: " + why;
    }
    if (a.beaten[0] >= 0) {
        out += std::string(" (rather than ") + at(kTable, a.beaten[0]).name;
        if (a.beaten[1] >= 0) {
            out += std::string(" or ") + at(kTable, a.beaten[1]).name;
        }
        out += ")";
    }
    return out;
}

std::vector<std::uint64_t> run(const Settings& settings, int days) {
    World world(settings);
    std::vector<std::uint64_t> sums;
    sums.reserve(static_cast<std::size_t>(days));
    for (int d = 0; d < days; ++d) {
        world.run_day();
        sums.push_back(world.checksum());
    }
    return sums;
}

}  // namespace minds
