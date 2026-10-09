#include "kd/demo/living.hpp"
#include <algorithm>
#include <limits>
#include "kd/chance/chance.hpp"
#include "kd/demo/crafting.hpp"
#include "kd/demo/learning.hpp"
#include "kd/demo/parts.hpp"

namespace kd::demo {
namespace {
constexpr time::Seconds kDay = time::kDay;
constexpr std::int64_t kFood = 4000000, kWater = 3000, kAwake = 129600;
constexpr std::uint32_t kUrgent = 1;
constexpr std::int64_t kUnavailable = -1000000;
using world::LivingAct;
bool in_rock(num::Offset p, const world::Habitat& rock) {
    return p.dx >= rock.rock_west && p.dx <= rock.rock_east && p.dy >= rock.rock_south && p.dy <= rock.rock_north;
}
bool line_clear(num::Offset a, num::Offset b, const world::Habitat& rock) {
    return world::camp_line_clear(a, b, rock);
}
const LivingRules& tuning(const data::Catalogue& c) {
    const auto& kinds = c.kind<LivingRules>();
    KD_CHECK(kinds.size() == 1, "Living camp needs one living tuning entry");
    return kinds[0];
}
void body(world::Life& l, const LivingRules& rules, LivingAct act, time::Seconds t) {
    const auto elapsed = std::max<time::Seconds>(0, t - l.settled);
    const auto food_units = elapsed * rules.food_day + l.food_remainder;
    const auto water_units = elapsed * rules.water_day + l.water_remainder;
    l.food -= food_units / kDay;
    l.water -= water_units / kDay;
    l.food_remainder = food_units % kDay;
    l.water_remainder = water_units % kDay;
    l.awake = std::clamp<std::int64_t>(l.awake + (act == LivingAct::rest ? -2 * elapsed : elapsed), 0, kAwake);
    l.settled = t;
}
std::int64_t share(const world::Activity& a, std::int64_t portion, time::Seconds t) {
    const auto done = std::clamp<time::Seconds>(t - a.start, 0, a.end - a.start);
    return portion * done / std::max<time::Seconds>(1, a.end - a.start);
}
}  // namespace
Living::Living(world::World& w) : rules_(tuning(w.catalogue())) {
    std::array<bool, 3> found{};
    const auto& uses = w.catalogue().kind<NeedUse>();
    for (std::uint32_t i = 0; i < uses.size(); ++i) {
        const auto& use = uses[i];
        const auto n = static_cast<std::size_t>(use.need);
        KD_CHECK(!found[n] && use.use.game > 0 && use.gather.game > 0, "Need affordances must be unique and timed");
        found[n] = true;
        uses_[n] = use;
    }
    KD_CHECK(std::all_of(found.begin(), found.end(), [](bool v) { return v; }),
             "Living camp needs its three body affordances");
    w.set_system(ecs::Family::person, *this);
    w.set_system(ecs::Family::place, *this);
    w.set_command_taker(*this);
}
std::array<std::int64_t, 3> Living::needs(const world::Life& l) {
    return {l.food * 100 / kFood, l.water * 100 / kWater, 100 - l.awake * 100 / kAwake};
}
world::Life Living::sample(world::Life l, const world::Activity& a, time::Seconds t) const {
    t = std::clamp(t, a.start, a.end);
    const auto act = static_cast<LivingAct>(a.what);
    body(l, rules_, act, t);
    const auto amount = std::max<std::int64_t>(0, share(a, l.portion, t) - l.applied);
    if (act == LivingAct::eat) {
        const auto nutrient = amount * l.food_factor_ppm + l.nutrient_remainder;
        l.food += nutrient / 1000000;
        l.nutrient_remainder = nutrient % 1000000;
        l.carried_food -= amount;
        const auto berry_water =
            amount * (l.meal_item.value == 0 ? uses_[0].water_per_kg : l.water_ml_per_kg) + l.food_water_remainder;
        l.water += berry_water / 1000000;
        l.food_water_remainder = berry_water % 1000000;  // raw berries: 0.8 L per kg
    } else if (act == LivingAct::drink) {
        l.water += amount;
        l.allocated_water -= amount;
    }
    // Intake and depletion belong to the same elapsed interval. Clamp only their net result.
    l.food = std::clamp<std::int64_t>(l.food, 0, kFood);
    l.water = std::clamp<std::int64_t>(l.water, 0, kWater);
    return l;
}
std::vector<num::Point> Living::route(const world::World& w, ecs::Id camp, num::Point from, num::Point to) {
    const auto h = w.beings().handle(camp);
    const auto& raw = w.beings().raw();
    const auto centre = raw.get<world::Place>(h).at;
    const auto& patch = raw.get<world::Camp>(h);
    const auto& rock = raw.get<world::Habitat>(h);
    const auto a = w.torus().offset(centre, from), b = w.torus().offset(centre, to);
    const auto inside = [&](num::Offset p) {
        return std::abs(p.dx) <= patch.half_width_cm && std::abs(p.dy) <= patch.half_height_cm;
    };
    if (!inside(a) || !inside(b) || in_rock(a, rock) || in_rock(b, rock)) return {};
    if (line_clear(a, b, rock)) return {to};
    const auto cols = patch.half_width_cm * 2 / 100 + 1, rows = patch.half_height_cm * 2 / 100 + 1;
    if (cols * rows > 10000) return {};
    const auto point = [&](std::int64_t index) {
        return num::Offset{index % cols * 100 - patch.half_width_cm, index / cols * 100 - patch.half_height_cm};
    };
    const auto cell = [&](num::Offset p) {
        return std::clamp((p.dy + patch.half_height_cm + 50) / 100, std::int64_t{0}, rows - 1) * cols +
               std::clamp((p.dx + patch.half_width_cm + 50) / 100, std::int64_t{0}, cols - 1);
    };
    const auto first = cell(a), last = cell(b);
    if (!line_clear(a, point(first), rock) || !line_clear(point(last), b, rock)) return {};
    std::vector<std::int64_t> previous(static_cast<std::size_t>(cols * rows), -1), queue{first};
    previous[static_cast<std::size_t>(first)] = first;
    for (std::size_t head = 0; head < queue.size() && previous[static_cast<std::size_t>(last)] < 0; ++head) {
        const auto at = queue[head];
        for (const auto step : std::array<num::Offset, 4>{{{0, 1}, {1, 0}, {0, -1}, {-1, 0}}}) {
            const auto x = at % cols + step.dx, y = at / cols + step.dy;
            if (x < 0 || y < 0 || x >= cols || y >= rows) continue;
            const auto next = y * cols + x;
            if (previous[static_cast<std::size_t>(next)] >= 0 || !line_clear(point(at), point(next), rock)) continue;
            previous[static_cast<std::size_t>(next)] = at;
            queue.push_back(next);
        }
    }
    if (previous[static_cast<std::size_t>(last)] < 0) return {};
    std::vector<num::Point> reversed{to};
    for (auto at = last; at != first; at = previous[static_cast<std::size_t>(at)])
        reversed.push_back(w.torus().moved(centre, point(at)));
    reversed.push_back(w.torus().moved(centre, point(first)));
    std::reverse(reversed.begin(), reversed.end());
    // Skip visible intermediate vertices, without ever crossing solid rock.
    std::size_t furthest = 0;
    for (std::size_t i = 0; i < reversed.size(); ++i) {
        if (line_clear(a, w.torus().offset(centre, reversed[i]), rock))
            furthest = i;
        else
            break;
    }
    reversed.erase(reversed.begin(), reversed.begin() + static_cast<std::ptrdiff_t>(furthest));
    return reversed;
}
bool Living::visible(const world::World& w, ecs::Id camp, num::Point from, num::Point to) {
    const auto h = w.beings().handle(camp);
    const auto centre = w.beings().raw().get<world::Place>(h).at;
    const auto& rock = w.beings().raw().get<world::Habitat>(h);
    return line_clear(w.torus().offset(centre, from), w.torus().offset(centre, to), rock);
}
void Living::start(world::World& w) {
    auto& raw = w.beings().raw();
    w.beings().each([&](ecs::Id id, world::Beings::Handle h) {
        if (raw.all_of<world::Camp>(h) && !raw.all_of<world::Dreams>(h)) raw.emplace<world::Dreams>(h);
        if (raw.all_of<world::Person>(h) && !raw.all_of<world::Dream>(h)) raw.emplace<world::Dream>(h);
        if (const auto* facts = raw.try_get<world::Camp>(h); facts && !raw.all_of<world::Habitat>(h)) {
            auto& env = raw.emplace<world::Habitat>(h);
            env.food_cap_mg = facts->food_mg;
            env.water_cap_ml = facts->water_ml;
            env.renewed_at = w.frontier();
            w.schedule(id, world::kActivitySlot, w.frontier() + 3600);
        }
        if (id.family() != ecs::Family::person || raw.all_of<world::Life>(h)) return;
        const auto home = raw.get<Home>(h).camp;
        const auto ch = w.beings().handle(home);
        const auto shelter = raw.get<world::Camp>(ch).shelter_at;
        auto& l = raw.emplace<world::Life>(h);
        l.settled = w.frontier();
        l.decision_at = w.frontier();
        const chance::Draws draws(w.seed(), chance::name("living"), id.value, w.frontier(),
                                  chance::name("starting needs"));
        l.food = draws.between(0, 3200000, kFood);
        l.water = draws.between(1, 2400, kWater);
        l.awake = draws.between(2, 0, 36000);
        l.known_at[2] = shelter;
        l.seen[2] = w.frontier();
        l.explore_at = raw.get<world::Place>(h).at;
        l.use_at = l.explore_at;
        auto& a = raw.get<world::Activity>(h);
        const auto at = raw.get<world::Place>(h).at;
        a = {0, w.frontier(), w.frontier() + 1, at, at};
        w.schedule(id, world::kActivitySlot, a.end);
    });
}
void Living::opened(world::World& w) {
    start(w);
}
namespace {
bool place_exists(const world::World& w, world::Beings::Handle home, std::int64_t subject) {
    const auto& env = w.beings().raw().get<world::Habitat>(home);
    return subject == 2 || (subject == 0 && env.food_cap_mg > 0) || (subject == 1 && env.water_cap_ml > 0);
}
void dream_night(world::Dreams& ledger, std::int64_t night) {
    if (ledger.night != night) {
        ledger.night = night;
        ledger.sent.fill(0);
    }
}
}  // namespace
std::string Living::dream_problem(const world::World& w, ecs::Id person, std::int64_t subject, time::Seconds at) {
    const auto h = w.beings().find(person);
    const auto& raw = w.beings().raw();
    if (!h || !raw.all_of<world::Person, world::Life, Home>(*h)) return "That person is gone";
    if (subject < 0 || subject > 2 || raw.get<world::Life>(*h).source[static_cast<std::size_t>(subject)] == 0)
        return "They haven't noticed that place";
    const auto ch = w.beings().find(raw.get<Home>(*h).camp);
    if (!ch || !raw.all_of<world::Dreams, world::Habitat>(*ch)) return "That camp is gone";
    if (!place_exists(w, *ch, subject)) return "That place is gone";
    const auto& ledger = raw.get<world::Dreams>(*ch);
    const auto pending =
        std::count_if(ledger.acts.begin(), ledger.acts.end(), [](const auto& a) { return a.status == 1; });
    if (std::any_of(ledger.acts.begin(), ledger.acts.end(),
                    [&](const auto& a) { return a.person == person.value && a.status == 1; }))
        return "A dream already waits for their sleep";
    if (pending >= 3) return "Three dreams are already waiting";
    if (ledger.night == night(at < 0 ? w.frontier() : at)) {
        if (std::find(ledger.sent.begin(), ledger.sent.end(), person.value) != ledger.sent.end())
            return "Already dreamt tonight; ready after 06:00";
        if (ledger.sent.back() != 0) return "Three dreams tonight; ready after 06:00";
    }
    return {};
}
void Living::place_dream(world::Context& c, world::Beings::Handle h, std::int64_t subject, num::Point place) {
    auto& thought = c.world().beings().raw().get<world::Dream>(h);
    thought.night = night(c.now());
    thought.at = c.now();
    thought.until = c.now() + kDreamLife;
    thought.subject = subject;
    thought.place = place;
    thought.visit_at = -1;
    // This same ordinary thought is the only path for natural and sent dreams. No sender enters a mind.
    c.record(150, c.world().beings().id_of(h).value, static_cast<std::uint64_t>(subject));
}
void Living::command(world::Context& c, const world::Command& cmd) {
    if (cmd.what != kPlaceDream || cmd.b > 2) return;
    const ecs::Id person{cmd.a};
    if (!dream_problem(c.world(), person, static_cast<std::int64_t>(cmd.b), c.now()).empty()) return;
    const auto h = c.world().beings().handle(person);
    auto& raw = c.world().beings().raw();
    const auto camp = raw.get<Home>(h).camp;
    auto& ledger = raw.get<world::Dreams>(c.world().beings().handle(camp));
    const auto subject = static_cast<std::int64_t>(cmd.b);
    world::DreamAct act;
    act.number = cmd.number;
    act.person = cmd.a;
    act.requested = cmd.at;
    act.received = c.now();
    act.subject = subject;
    act.place = raw.get<world::Life>(h).known_at[static_cast<std::size_t>(subject)];
    const auto position =
        std::upper_bound(ledger.acts.begin(), ledger.acts.end(), act.number,
                         [](std::uint64_t number, const world::DreamAct& old) { return number < old.number; });
    ledger.acts.insert(position, act);
    if (raw.get<world::Activity>(h).what == static_cast<std::uint8_t>(LivingAct::rest)) {
        sleep_dream(c, h, camp);
        // Publish the new thought at this event, retaining the same sleep and its scheduled end.
        c.moved(person);
    }
}
void Living::sleep_dream(world::Context& c, world::Beings::Handle h, ecs::Id camp) {
    auto& raw = c.world().beings().raw();
    auto& thought = raw.get<world::Dream>(h);
    const auto home = c.world().beings().handle(camp);
    auto& ledger = raw.get<world::Dreams>(home);
    const auto id = c.world().beings().id_of(h);
    const auto tonight = night(c.now());
    dream_night(ledger, tonight);
    for (auto& act : ledger.acts) {
        if (act.person != id.value || act.status != 1) continue;
        act.executed = c.now();
        if (!place_exists(c.world(), home, act.subject)) {
            act.status = 3;
            act.reason = 1;
            break;
        }
        const auto free = std::find(ledger.sent.begin(), ledger.sent.end(), 0);
        if (free == ledger.sent.end() ||
            std::find(ledger.sent.begin(), ledger.sent.end(), id.value) != ledger.sent.end()) {
            act.status = 3;
            act.reason = 2;
            break;
        }
        *free = id.value;
        act.status = 2;
        act.until = c.now() + kDreamLife;
        place_dream(c, h, act.subject, act.place);
        return;
    }
    if (thought.night == tonight) return;
    thought.night = tonight;
    // Scoped natural place dreams: weak dreams are unkept; occasionally the same strong thought remains.
    const chance::Draws draws(c.world().seed(), chance::name("living"), id.value, tonight, chance::name("place dream"));
    if (draws.between(0, 0, 59) != 0) return;
    const auto& life = raw.get<world::Life>(h);
    const auto need = needs(life);
    std::int64_t subject = -1;
    for (std::size_t i = 0; i < 3; ++i) {
        if (life.source[i] == 0) continue;
        if (subject < 0 || need[i] < need[static_cast<std::size_t>(subject)] ||
            (need[i] == need[static_cast<std::size_t>(subject)] &&
             life.seen[i] > life.seen[static_cast<std::size_t>(subject)]))
            subject = static_cast<std::int64_t>(i);
    }
    if (subject >= 0) place_dream(c, h, subject, life.known_at[static_cast<std::size_t>(subject)]);
}
void Living::dream_consequence(world::Context& c, world::Beings::Handle h, ecs::Id camp, bool arrived) {
    auto& raw = c.world().beings().raw();
    auto& thought = raw.get<world::Dream>(h);
    const auto& life = raw.get<world::Life>(h);
    if (thought.at < 0 || thought.until <= c.now()) return;
    const auto id = c.world().beings().id_of(h);
    if (arrived) thought.visit_at = c.now();
    auto& acts = raw.get<world::Dreams>(c.world().beings().handle(camp)).acts;
    for (auto& act : acts) {
        if (act.status != 2 || act.person != id.value || act.executed != thought.at || act.subject != thought.subject)
            continue;
        if (arrived)
            act.visited_at = c.now();
        else if (act.decision_at == -1) {
            act.decision_at = c.now();
            act.choice = life.goal;
            act.pull = thought.decision_pull;
        }
    }
}
num::Point Living::use_spot(const world::World& w, world::Beings::Handle h, ecs::Id camp, std::size_t need) const {
    const auto& raw = w.beings().raw();
    const auto& l = raw.get<world::Life>(h);
    const auto id = w.beings().id_of(h);
    const auto here = raw.get<world::Place>(h).at;
    const auto act = static_cast<LivingAct>(raw.get<world::Activity>(h).what);
    if ((need == 1 && act == LivingAct::drink) || (need == 2 && (act == LivingAct::rest || act == LivingAct::eat)))
        return here;
    const auto ch = w.beings().handle(camp);
    const auto centre = raw.get<world::Place>(ch).at;
    const auto& patch = raw.get<world::Camp>(ch);
    const auto anchor = l.known_at[need];
    const auto radius = uses_[need].area / 10;
    const chance::Draws draws(w.seed(), chance::name("living"), id.value, 0, chance::name("use position"));
    for (std::uint64_t attempt = 0; attempt < 8; ++attempt) {
        const auto offset = w.torus().offset(centre, anchor);
        const auto target =
            w.torus().moved(centre, {std::clamp(offset.dx + draws.between(need * 16 + attempt * 2, -radius, radius),
                                                -patch.half_width_cm, patch.half_width_cm),
                                     std::clamp(offset.dy + draws.between(need * 16 + attempt * 2 + 1, -radius, radius),
                                                -patch.half_height_cm, patch.half_height_cm)});
        if (!route(w, camp, here, target).empty()) return target;
    }
    return anchor;
}
void Living::notice(world::Context& c, world::Beings::Handle h, ecs::Id camp) {
    auto& raw = c.world().beings().raw();
    auto& l = raw.get<world::Life>(h);
    if (c.now() - l.notice_at < 3600) return;
    l.notice_at = c.now();
    const auto ch = c.world().beings().handle(camp);
    const auto& facts = raw.get<world::Camp>(ch);
    const auto here = raw.get<world::Place>(h).at;
    const auto clock = c.now() % kDay;
    const std::int64_t range = clock >= 6 * time::kHour && clock < 20 * time::kHour ? 5000 : 500;
    const std::array<num::Point, 3> sites{facts.food_at, facts.water_at, facts.shelter_at};
    const std::array<std::int64_t, 3> amounts{facts.food_mg, facts.water_ml, 1};
    for (std::size_t i = 0; i < sites.size(); ++i) {
        if (c.world().torus().squared_distance(here, sites[i]) > range * range ||
            !visible(c.world(), camp, here, sites[i]))
            continue;
        l.known_at[i] = sites[i];
        l.known_amount[i] = amounts[i];
        l.seen[i] = c.now();
        l.source[i] = 1;
    }
}
void Living::begin(world::Context& c, world::Beings::Handle h, LivingAct what, time::Seconds takes, num::Point to) {
    auto& raw = c.world().beings().raw();
    const auto id = c.world().beings().id_of(h);
    auto& l = raw.get<world::Life>(h);
    auto& a = raw.get<world::Activity>(h);
    if (auto* mind = raw.try_get<world::Knowledge>(h); mind && what != LivingAct::watch_craft) mind->watching = {};
    const auto from = raw.get<world::Place>(h).at;
    a = {static_cast<std::uint8_t>(what), c.now(), c.now() + std::max<time::Seconds>(1, takes), from, to};
    l.applied = 0;
    c.schedule(id, world::kActivitySlot, a.end);
    c.cancel(id, kUrgent);
    auto urgent = a.end;
    if (what != LivingAct::eat && l.food >= kFood / 5)
        urgent = std::min(urgent, c.now() + ((l.food - kFood / 5) * kDay - l.food_remainder) / rules_.food_day + 1);
    if (what != LivingAct::drink && what != LivingAct::eat && l.water >= kWater / 5)
        urgent = std::min(urgent, c.now() + ((l.water - kWater / 5) * kDay - l.water_remainder) / rules_.water_day + 1);
    // Exhaustion must still interrupt work that began after the earlier fatigue warning.
    if (what != LivingAct::rest) urgent = std::min(urgent, c.now() + kAwake - l.awake);
    if (what != LivingAct::rest && l.awake <= kAwake * 4 / 5)
        urgent = std::min(urgent, c.now() + kAwake * 4 / 5 - l.awake + 1);
    if (urgent < a.end) c.schedule(id, kUrgent, std::max(c.now() + 1, urgent));
    if (what == LivingAct::rest) sleep_dream(c, h, raw.get<Home>(h).camp);
    c.moved(id);
}
void Living::choose(world::Context& c, world::Beings::Handle h, ecs::Id camp) {
    auto& raw = c.world().beings().raw();
    auto& l = raw.get<world::Life>(h);
    notice(c, h, camp);
    l.decision_needs = needs(l);
    l.decision_at = c.now();
    l.scores.fill(kUnavailable);
    l.unavailable.fill(0);
    l.benefit.fill(0);
    l.cost_seconds.fill(0);
    l.scores[3] = 0;
    const auto here = raw.get<world::Place>(h).at;
    for (std::size_t i = 0; i < 3; ++i) {
        if (l.blocked_until[i] > c.now()) {
            l.unavailable[i] = 5;
            continue;
        }
        if (i == 0 && l.gathering_skill == 0) {
            l.unavailable[i] = 4;
            continue;
        }
        if (l.source[i] == 0) {
            l.unavailable[i] = 1;
            continue;
        }
        if (l.known_amount[i] <= 0 && !(i == 0 && l.carried_food > 0)) {
            l.unavailable[i] = 2;
            continue;
        }
        const auto destination =
            i == 2 && l.awake >= kAwake ? here : use_spot(c.world(), h, camp, i == 0 && l.carried_food > 0 ? 2 : i);
        const auto path = route(c.world(), camp, here, destination);
        if (path.empty()) {
            l.unavailable[i] = 3;
            l.blocked_until[i] = c.now() + 3600;
            continue;
        }
        std::int64_t distance = 0;
        auto previous = here;
        for (const auto point : path) {
            distance += c.world().torus().distance(previous, point);
            previous = point;
        }
        const auto urgency = i == 2 && l.awake >= kAwake ? 200 : std::max<std::int64_t>(0, 80 - l.decision_needs[i]);
        const auto available =
            i == 2 ? uses_[i].amount() : (i == 0 && l.carried_food > 0 ? l.carried_food : l.known_amount[i]);
        const auto benefit = std::min(uses_[i].benefit * std::min(available, uses_[i].amount()) / uses_[i].amount(),
                                      100 - l.decision_needs[i]);
        const auto work = i == 0 && l.carried_food == 0 ? uses_[i].gather.game : 0;
        l.benefit[i] = benefit;
        l.cost_seconds[i] = uses_[i].use.game + work + (distance * 10 + rules_.speed - 1) / rules_.speed;
        l.scores[i] = urgency * benefit * 10 - l.cost_seconds[i] / 60;
    }
    auto& thought = raw.get<world::Dream>(h);
    thought.decision_pull = 0;
    thought.decision_subject = -1;
    const bool mild = *std::min_element(l.decision_needs.begin(), l.decision_needs.end()) >= 20 && l.awake < kAwake;
    const bool dreaming = mild && thought.at >= 0 && thought.until > c.now() && thought.subject >= 0;
    bool visiting = false;
    if (dreaming) {
        const auto subject = static_cast<std::size_t>(thought.subject);
        if (l.scores[subject] != kUnavailable && !(subject == 0 && l.carried_food > 0)) l.scores[subject] += kDreamPull;
        const auto path = route(c.world(), camp, here, thought.place);
        if (!path.empty() && here != thought.place) {
            std::int64_t distance = 0;
            auto previous = here;
            for (const auto point : path) {
                distance += c.world().torus().distance(previous, point);
                previous = point;
            }
            l.scores[3] = kDreamPull - (distance * 10 / rules_.speed) / 60;
            visiting = l.scores[3] > 0;
        }
    }
    l.goal = 3;
    for (std::uint8_t i = 0; i < 3; ++i)
        if (l.scores[i] > l.scores[l.goal]) l.goal = i;
    if (dreaming &&
        ((l.goal == 3 && visiting) || (l.goal == thought.subject && !(l.goal == 0 && l.carried_food > 0)))) {
        thought.decision_pull = kDreamPull;
        thought.decision_subject = thought.subject;
    }
    if (Crafting::choose(*this, c, h)) return;
    dream_consequence(c, h, camp, false);
    if (l.goal < 3)
        l.use_at = l.goal == 2 && l.awake >= kAwake
                       ? here
                       : use_spot(c.world(), h, camp, l.goal == 0 && l.carried_food > 0 ? 2 : l.goal);
    if (l.goal == 3 && visiting) l.explore_at = thought.place;
    if (l.goal == 3 && !visiting) {
        const auto id = c.world().beings().id_of(h);
        const chance::Draws draws(c.world().seed(), chance::name("living"), id.value, c.now(),
                                  chance::name("nearby ground"));
        const auto ch = c.world().beings().handle(camp);
        const auto centre = raw.get<world::Place>(ch).at;
        const auto& facts = raw.get<world::Camp>(ch);
        l.explore_at = here;
        for (std::uint64_t attempt = 0; attempt < 8; ++attempt) {
            const auto target = c.world().torus().moved(
                centre, {draws.between(attempt * 2, -facts.half_width_cm, facts.half_width_cm),
                         draws.between(attempt * 2 + 1, -facts.half_height_cm, facts.half_height_cm)});
            if (!route(c.world(), camp, here, target).empty()) {
                l.explore_at = target;
                break;
            }
        }
    }
    std::uint64_t packed = 0;
    for (std::size_t n = 0; n < 3; ++n) packed |= static_cast<std::uint64_t>(l.decision_needs[n]) << (n * 7U);
    const auto id = c.world().beings().id_of(h);
    c.record(100U + l.goal, id.value, packed);
    for (std::uint32_t n = 0; n < 4; ++n) c.record(110U + n, id.value, static_cast<std::uint64_t>(l.scores[n]));
    for (std::uint32_t n = 0; n < 3; ++n) {
        c.record(120U + n, id.value, static_cast<std::uint64_t>(l.benefit[n]));
        c.record(130U + n, id.value, static_cast<std::uint64_t>(l.cost_seconds[n]));
    }
    const auto exclusions = static_cast<std::uint64_t>(l.unavailable[0]) |
                            (static_cast<std::uint64_t>(l.unavailable[1]) << 4U) |
                            (static_cast<std::uint64_t>(l.unavailable[2]) << 8U);
    c.record(140, id.value, exclusions);
    continue_goal(c, h, camp);
}
void Living::continue_goal(world::Context& c, world::Beings::Handle h, ecs::Id camp) {
    auto& raw = c.world().beings().raw();
    auto& l = raw.get<world::Life>(h);
    const auto here = raw.get<world::Place>(h).at;
    const auto& thought = raw.get<world::Dream>(h);
    if (thought.at >= 0 && thought.until > c.now() && thought.subject >= 0) {
        const auto offset = c.world().torus().offset(thought.place, here);
        const auto radius = uses_[static_cast<std::size_t>(thought.subject)].area / 10;
        const bool in_site = std::abs(offset.dx) <= radius && std::abs(offset.dy) <= radius;
        const bool reached =
            l.goal == 3 ? here == thought.place : l.goal == thought.subject && here == l.use_at && in_site;
        if (reached) dream_consequence(c, h, camp, true);
    }
    if (l.goal == 3) {
        const auto path = route(c.world(), camp, here, l.explore_at);
        l.portion = 0;
        if (here != l.explore_at && !path.empty()) {
            const auto length = c.world().torus().distance(here, path.front()) * 10;
            begin(c, h, LivingAct::walk, (length + rules_.speed - 1) / rules_.speed, path.front());
        } else {
            ecs::Id target{};
            c.world().beings().each([&](ecs::Id person, world::Beings::Handle other) {
                const auto* work = raw.try_get<world::Work>(other);
                if (other == h || !work || work->state != 2 || raw.get<Home>(other).camp != camp) return;
                const auto at = raw.get<world::Activity>(other).at(c.world().torus(), c.now());
                if (Learning::can_watch(c.world(), camp, here, at, c.now()) && (target.value == 0 || person < target))
                    target = person;
            });
            if (target.value != 0) {
                raw.get<world::Knowledge>(h).watching = target;
                const auto end = raw.get<world::Work>(c.world().beings().handle(target)).end;
                begin(c, h, LivingAct::watch_craft, std::min<time::Seconds>(3600, end - c.now()), here);
            } else
                begin(c, h, LivingAct::watch, 3600, here);
        }
        return;
    }
    const auto ch = c.world().beings().handle(camp);
    auto& facts = raw.get<world::Camp>(ch);
    auto& env = raw.get<world::Habitat>(ch);
    const auto goal = static_cast<std::size_t>(l.goal);
    const auto destination = l.use_at;
    if (here != destination) {
        const auto path = route(c.world(), camp, here, destination);
        if (path.empty()) {
            l.blocked_until[goal] = c.now() + 3600;
            choose(c, h, camp);
            return;
        }
        const auto speed = (l.carried_food > 0 ? rules_.loaded_speed : rules_.speed) *
                           (l.food < kFood * 3 / 10 || l.water < kWater * 3 / 10 ? 8 : 10) / 10;
        const auto length = c.world().torus().distance(here, path.front()) * 10;
        l.portion = 0;
        begin(c, h, l.carried_food > 0 ? LivingAct::carry : LivingAct::walk, (length + speed - 1) / speed,
              path.front());
        return;
    }
    l.applied = 0;
    if (goal == 0 && l.carried_food > 0) {
        if (l.meal_item.value != 0) {
            const auto& item = c.world().things().raw().get<world::Item>(c.world().things().handle(l.meal_item));
            const auto actual = Crafting::characteristics(c.world().catalogue(), item);
            l.food_factor_ppm = actual[8] * 500000;
            l.water_ml_per_kg = actual[9] * 200;
        }
        l.portion = std::min(l.carried_food, uses_[goal].amount());
        begin(c, h, LivingAct::eat, uses_[goal].use.game, here);
    } else if (goal == 0) {
        l.known_amount[0] = facts.food_mg;
        l.seen[0] = c.now();
        l.source[0] = 3;
        if (facts.food_mg == 0) {
            choose(c, h, camp);
            return;
        }
        l.portion = std::min(facts.food_mg, uses_[goal].amount());
        begin(c, h, LivingAct::gather, uses_[goal].gather.game, here);
    } else if (goal == 1) {
        l.known_amount[1] = facts.water_ml;
        l.seen[1] = c.now();
        l.source[1] = 3;
        if (facts.water_ml == 0) {
            choose(c, h, camp);
            return;
        }
        l.allocated_water = std::min(facts.water_ml, uses_[goal].amount());
        l.portion = l.allocated_water;
        facts.water_ml -= l.allocated_water;
        env.water_taken += l.allocated_water;
        begin(c, h, LivingAct::drink, uses_[goal].use.game, here);
    } else {
        l.portion = uses_[goal].amount();
        begin(c, h, LivingAct::rest, uses_[goal].use.game, here);
    }
}
void Living::settle(world::Context& c, world::Beings::Handle h, ecs::Id camp, bool interrupted) {
    auto& raw = c.world().beings().raw();
    auto& l = raw.get<world::Life>(h);
    auto& a = raw.get<world::Activity>(h);
    const auto act = static_cast<LivingAct>(a.what);
    const auto elapsed = c.now() - l.settled;
    const auto projected = sample(l, a, c.now());
    l.food = projected.food;
    l.water = projected.water;
    l.awake = projected.awake;
    l.settled = projected.settled;
    l.food_remainder = projected.food_remainder;
    l.water_remainder = projected.water_remainder;
    l.food_water_remainder = projected.food_water_remainder;
    l.nutrient_remainder = projected.nutrient_remainder;
    const auto amount = std::max<std::int64_t>(0, share(a, l.portion, c.now()) - l.applied);
    const auto ch = c.world().beings().handle(camp);
    auto& facts = raw.get<world::Camp>(ch);
    auto& env = raw.get<world::Habitat>(ch);
    if (act == LivingAct::eat) {
        l.carried_food -= amount;
        l.memory_kind = a.what;
        l.memory_at = c.now();
        l.memory_amount = amount;
        Crafting::settle_meal(c, h, amount, interrupted || c.now() == a.end);
    }
    if (interrupted && act != LivingAct::eat && l.meal_item.value != 0) Crafting::settle_meal(c, h, 0, true);
    if (act == LivingAct::drink) {
        l.allocated_water -= amount;
        l.memory_kind = a.what;
        l.memory_at = c.now();
        l.memory_amount = amount;
        if (interrupted) {
            const auto returned =
                std::min(l.allocated_water, std::max<std::int64_t>(0, env.water_cap_ml - facts.water_ml));
            facts.water_ml += returned;
            env.water_taken -= returned;
            env.water_spilled += l.allocated_water - returned;
            l.allocated_water = 0;
        }
    }
    if (act == LivingAct::gather) {
        const auto take = std::min(amount, facts.food_mg);
        facts.food_mg -= take;
        env.food_taken += take;
        l.carried_food += take;
        l.known_amount[0] = facts.food_mg;
        l.seen[0] = c.now();
        l.source[0] = 3;
    }
    if (act == LivingAct::rest) {
        l.memory_kind = a.what;
        l.memory_at = c.now();
        l.memory_amount = elapsed;
    }
    l.applied += amount;
    raw.get<world::Place>(h).at = a.at(c.world().torus(), c.now());
    if (const auto* work = raw.try_get<world::Work>(h)) {
        for (const auto& r : work->inputs)
            if (r.picked) {
                c.world().things().raw().get<world::Place>(c.world().things().handle(r.item)).at =
                    raw.get<world::Place>(h).at;
                c.item_changed(r.item);
            }
    }
}
void Living::renew(world::Context& c, world::Beings::Handle h) {
    auto& raw = c.world().beings().raw();
    auto& facts = raw.get<world::Camp>(h);
    auto& env = raw.get<world::Habitat>(h);
    auto& ledger = raw.get<world::Dreams>(h);
    for (auto& act : ledger.acts) {
        if (act.status == 1 && !c.world().beings().find(ecs::Id{act.person})) {
            act.status = 3;
            act.reason = 3;
            act.executed = c.now();
        }
    }
    const auto hours = (c.now() - env.renewed_at) / 3600;
    const auto water = std::max<std::int64_t>(
        0, std::min({rules_.spring_hour * hours, env.upstream_ml, env.water_cap_ml - facts.water_ml}));
    env.upstream_ml -= water;
    env.water_added += water;
    facts.water_ml += water;
    const auto grown = std::max<std::int64_t>(
        0, std::min({rules_.fruit_hour * hours, env.crop_budget_mg, env.root_water_ml * 1000000 / rules_.fruit_water,
                     env.food_cap_mg - facts.food_mg}));
    const auto used = (grown * rules_.fruit_water + 999999) / 1000000;
    env.root_water_ml -= used;
    env.crop_budget_mg -= grown;
    env.food_grown += grown;
    facts.food_mg += grown;
    env.renewed_at = c.now();
    const auto id = c.world().beings().id_of(h);
    c.schedule(id, world::kActivitySlot, c.now() + 3600);
}
void Living::handle(world::Context& c, const event::Event& e) {
    const ecs::Id id{e.key.owner};
    const auto h = c.world().beings().handle(id);
    if (id.family() == ecs::Family::place) {
        renew(c, h);
        return;
    }
    const auto camp = c.world().beings().raw().get<Home>(h).camp;
    c.touch(camp);
    auto& raw = c.world().beings().raw();
    const auto old = static_cast<LivingAct>(raw.get<world::Activity>(h).what);
    const auto interrupted = e.slot == kUrgent;
    Learning::observe(c, h);
    settle(c, h, camp, interrupted);
    c.cancel(id, world::kActivitySlot);
    c.cancel(id, kUrgent);
    if (raw.all_of<world::Work>(h)) {
        const bool working = raw.get<world::Work>(h).state == 2;
        Crafting::settle(*this, c, h, interrupted, e.slot == 2);
        if (raw.get<world::Work>(h).state == 2) return;
        if (!interrupted && !working && Crafting::continue_work(*this, c, h)) return;
    }
    if (!interrupted && (old == LivingAct::walk || old == LivingAct::carry ||
                         (old == LivingAct::gather && raw.get<world::Life>(h).carried_food > 0))) {
        if (old == LivingAct::gather) raw.get<world::Life>(h).use_at = use_spot(c.world(), h, camp, 2);
        continue_goal(c, h, camp);
    } else
        choose(c, h, camp);
}
void Living::bounds(const world::World& w, time::Seconds /*a*/, time::Seconds /*b*/, std::span<const ecs::Id> owners,
                    std::vector<world::Bound>& out) const {
    for (const auto id : owners) {
        const auto h = w.beings().handle(id);
        const auto camp = id.family() == ecs::Family::place ? id : w.beings().raw().get<Home>(h).camp;
        const auto ch = w.beings().handle(camp);
        const auto centre = w.beings().raw().get<world::Place>(ch).at;
        out.push_back({id, centre, 10000});
    }
}
void Living::near(const world::World& w, time::Seconds /*a*/, time::Seconds /*b*/, const world::Bound& bound,
                  std::vector<ecs::Id>& out) const {
    w.beings().each([&](ecs::Id id, world::Beings::Handle h) {
        if (!w.beings().raw().any_of<world::Life, world::Habitat>(h)) return;
        const auto camp = id.family() == ecs::Family::place ? id : w.beings().raw().get<Home>(h).camp;
        const auto ch = w.beings().handle(camp);
        const auto centre = w.beings().raw().get<world::Place>(ch).at;
        if (w.torus().squared_distance(centre, bound.centre) <= (bound.radius + 10000) * (bound.radius + 10000))
            out.push_back(id);
    });
}
}  // namespace kd::demo
