#include "crowd_core.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <utility>

#if defined(__linux__) || defined(__ANDROID__)
#include <sched.h>
#endif

#include "kd/demo/fire.hpp"
#include "kd/demo/parts.hpp"
#include "trace.hpp"

namespace kd::view {

void place(const num::Torus& torus, const world::Activity& way, double t, num::Point origin, double& east,
           double& north) {
    const double whole = std::floor(t);
    const auto second = static_cast<time::Seconds>(whole);
    const num::Point here = way.at(torus, second);
    const num::Offset from_origin = torus.offset(origin, here);
    const num::Offset step = torus.offset(here, way.at(torus, second + 1));
    const double fraction = t - whole;
    east = static_cast<double>(from_origin.dx) + static_cast<double>(step.dx) * fraction;
    north = static_cast<double>(from_origin.dy) + static_cast<double>(step.dy) * fraction;
}

std::size_t Snapshot::way_index(std::size_t i, double t) const {
    std::uint32_t k = first[i];
    while (k + 1 < first[i + 1] &&
           static_cast<double>(changed_at.empty() ? ways[k + 1].start : changed_at[k + 1]) <= t) {
        ++k;
    }
    return k;
}
const world::Activity& Snapshot::way_at(std::size_t i, double t) const {
    return ways[way_index(i, t)];
}

const world::ItemWay* Snapshot::item_at(std::size_t i, double t) const {
    auto at = item_first[i];
    if (static_cast<double>(items[at].key.second) > t) return nullptr;
    while (at + 1 < item_first[i + 1] && static_cast<double>(items[at + 1].key.second) <= t) ++at;
    return &items[at];
}

std::optional<world::Thermal> Snapshot::thermal_at(std::size_t walker, double t, std::int64_t water_day) const {
    const auto k = way_index(walker, t);
    if (k >= thermals.size() || !thermals[k]) return std::nullopt;
    const auto camp = camp_ids[walkers[walker].camp];
    std::vector<demo::FireRules::HeatField> fires;
    for (std::size_t i = 0; i + 1 < item_first.size(); ++i) {
        const auto* saved = item_at(i, t);
        if (!saved || !saved->fire || saved->fire->hearth != camp || saved->fire->heat < 2) continue;
        const auto& f = *saved->fire;
        auto activity = world::Activity{0, 0, std::numeric_limits<time::Seconds>::max(), f.at, f.at};
        if (f.owner.value)
            for (std::size_t owner = 0; owner < walkers.size(); ++owner)
                if (walkers[owner].id == f.owner.value) activity = way_at(owner, t);
        fires.push_back({saved->id, activity});
    }
    const auto ambient = k < ambients.size() && ambients[k] ? ambients[k].value_or(world::Ambient{}).milli_c
                                                            : demo::FireRules::ambient(static_cast<time::Seconds>(t));
    return demo::FireRules::sample_thermal(thermals[k].value_or(world::Thermal{}), ways[k], world::World::kTorus,
                                           static_cast<time::Seconds>(t), water_day, ambient, fires);
}

CrowdStepper::CrowdStepper(demo::CrowdWorld& crowd) : crowd_(crowd), camp_ids_(crowd.camp_ids()) {
    const world::World& w = crowd_.world();
    const auto& raw = w.beings().raw();
    for (const ecs::Id id : camp_ids_) {
        camp_places_.push_back(raw.get<world::Place>(w.beings().handle(id)).at);
    }
    const std::vector<ecs::Id>& camps = camp_ids_;
    w.beings().each([&](ecs::Id id, world::Beings::Handle h) {
        if (id.family() != ecs::Family::marker && id.family() != ecs::Family::person) {
            return;
        }
        const ecs::Id camp = raw.get<demo::Home>(h).camp;
        const auto c = static_cast<std::uint32_t>(std::lower_bound(camps.begin(), camps.end(), camp) - camps.begin());
        ids_.push_back(id);
        const auto* person = raw.try_get<world::Person>(h);
        walkers_.push_back({id.value, person != nullptr ? 0 : raw.get<demo::MarkerKind>(h).kind, c,
                            person != nullptr ? std::optional<world::Person>(*person) : std::nullopt});
        trails_.push_back({raw.get<world::Activity>(h)});
        // Marker worlds have no body state. Avoid retaining a full empty Life for every historical way.
        if (crowd_.living() != nullptr) {
            const auto* life = raw.try_get<world::Life>(h);
            life_trails_.push_back({life ? std::optional<world::Life>(*life) : std::nullopt});
            const auto* thought = raw.try_get<world::Dream>(h);
            dream_trails_.push_back({thought ? std::optional<world::Dream>(*thought) : std::nullopt});
            change_trails_.push_back({raw.get<world::Activity>(h).start});
            const auto* work = raw.try_get<world::Work>(h);
            work_trails_.push_back({work ? std::optional<world::Work>(*work) : std::nullopt});
            const auto* thermal = raw.try_get<world::Thermal>(h);
            thermal_trails_.push_back({thermal ? std::optional<world::Thermal>(*thermal) : std::nullopt});
            const auto* ambient = raw.try_get<world::Ambient>(w.beings().handle(camp));
            ambient_trails_.push_back({ambient ? std::optional<world::Ambient>(*ambient) : std::nullopt});
            const auto* know = raw.try_get<world::Knowledge>(h);
            knowledge_trails_.push_back({know ? world::KnowledgeView::capture(*know) : world::KnowledgeView{}});
        }
    });
    w.things().each([&](ecs::Id id, world::Things::Handle h) {
        const auto* item = w.things().raw().try_get<world::Item>(h);
        if (!item) return;
        const auto* fire = w.things().raw().try_get<world::Fire>(h);
        const auto* timer = w.things().raw().try_get<world::HeatTimer>(h);
        mutable_item_trail(id).push_back({{w.frontier(), id.value, 0},
                                          0,
                                          id,
                                          w.things().raw().get<world::Place>(h),
                                          *item,
                                          fire ? std::optional<world::Fire>(*fire) : std::nullopt,
                                          timer ? std::optional<world::HeatTimer>(*timer) : std::nullopt});
    });
    crowd_.world().keep_item_ways(&item_ways_);
    crowd_.world().keep_history(&history_);
    crowd_.world().keep_ways(&ways_);
    // the first snapshot, so the screen has something to draw before the first batch
    fill(snapshots_.back());
    snapshots_.publish();
}

std::vector<world::ItemWay>& CrowdStepper::mutable_item_trail(ecs::Id id) {
    auto& trail = item_trails_[id];
    if (!trail)
        trail = std::make_shared<std::vector<world::ItemWay>>();
    else if (!trail.unique())
        trail = std::make_shared<std::vector<world::ItemWay>>(*trail);
    return *trail;
}

void CrowdStepper::fill(Snapshot& s) const {
    const auto& w = std::as_const(crowd_.world());
    s.frontier = w.frontier();
    s.walkers = walkers_;
    s.supplies.clear();
    s.habitats.clear();
    s.lives.clear();
    s.dreams.clear();
    s.changed_at.clear();
    s.works.clear();
    s.knowledge.clear();
    s.thermals.clear();
    s.ambients.clear();
    s.camp_ids = camp_ids_;
    s.items.clear();
    s.item_first.clear();
    s.craft_history.clear();
    s.item_archive = w.item_archive();
    s.archive_index = w.archive_index();
    for (const auto& [id, trail] : item_trails_) {
        (void)id;
        s.item_first.push_back(static_cast<std::uint32_t>(s.items.size()));
        s.items.append(trail);
    }
    s.item_first.push_back(static_cast<std::uint32_t>(s.items.size()));
    for (const ecs::Id id : camp_ids_) {
        const auto* camp = w.beings().raw().try_get<world::Camp>(w.beings().handle(id));
        if (camp != nullptr) s.supplies.push_back(*camp);
        const auto* habitat = w.beings().raw().try_get<world::Habitat>(w.beings().handle(id));
        if (habitat != nullptr) s.habitats.push_back(*habitat);
        const auto* history = w.beings().raw().try_get<world::CraftHistory>(w.beings().handle(id));
        if (history) {
            (void)history->public_index();
            s.craft_history.push_back(*history);
        }
    }
    s.ways.clear();
    s.first.clear();
    for (std::size_t i = 0; i < trails_.size(); ++i) {
        const auto& trail = trails_[i];
        if (!life_trails_.empty()) s.lives.append(life_trails_[i]);
        if (!dream_trails_.empty()) s.dreams.append(dream_trails_[i]);
        if (!work_trails_.empty()) s.works.append(work_trails_[i]);
        if (!knowledge_trails_.empty()) s.knowledge.append(knowledge_trails_[i]);
        if (!thermal_trails_.empty()) s.thermals.append(thermal_trails_[i]);
        if (!ambient_trails_.empty()) s.ambients.append(ambient_trails_[i]);
        if (!change_trails_.empty()) s.changed_at.append(change_trails_[i]);
        s.first.push_back(static_cast<std::uint32_t>(s.ways.size()));
        s.ways.append(trail);
    }
    s.first.push_back(static_cast<std::uint32_t>(s.ways.size()));
}

time::Seconds CrowdStepper::advance(time::Seconds frontier, time::Seconds goal) {
    {
        std::lock_guard lock(pin_mutex_);
        if (pin_asked_) {
            pin_asked_ = false;
#if defined(__linux__) || defined(__ANDROID__)
            cpu_set_t set;
            CPU_ZERO(&set);
            if (pin_.empty()) {
                for (int i = 0; i < CPU_SETSIZE; ++i) {
                    CPU_SET(i, &set);
                }
            } else {
                for (const int core : pin_) {
                    CPU_SET(core, &set);
                }
            }
            sched_setaffinity(0, sizeof set, &set);
#endif
        }
    }
    // each batch a section of the phone's trace, on the world's thread (PLT-04)
    const TraceSection section("kd batch");
    const auto started = std::chrono::steady_clock::now();
    const auto spent = [&] {
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
    };
    const std::uint64_t before = crowd_.world().events_run();
    // steps of a quarter of the batch at what the phone can do now, a game minute before that is known, and never
    // more than kStep, so a step from a quiet night into a busy day overruns little
    const double can = capacity_.load(std::memory_order_relaxed);
    const time::Seconds step =
        can > 0.0 ? std::clamp<time::Seconds>(std::llround(can * kBatchSeconds / 4.0), 1, kStep) : 60;
    time::Seconds reached = frontier;
    do {
        reached = crowd_.world().advance(reached, std::min(goal, reached + step));
    } while (reached < goal && spent() < kBatchSeconds);

    // the batch's ways join their walkers' trails, and those the screen has passed leave them
    for (const world::Way& way : ways_) {
        const auto i = static_cast<std::size_t>(std::lower_bound(ids_.begin(), ids_.end(), way.id) - ids_.begin());
        trails_[i].push_back(way.activity);
        if (!life_trails_.empty()) life_trails_[i].push_back(way.life);
        if (!dream_trails_.empty()) dream_trails_[i].push_back(way.dream);
        if (!work_trails_.empty()) work_trails_[i].push_back(way.work);
        if (!knowledge_trails_.empty()) knowledge_trails_[i].push_back(way.knowledge);
        if (!change_trails_.empty()) change_trails_[i].push_back(way.key.second);
        if (!thermal_trails_.empty()) thermal_trails_[i].push_back(way.thermal);
        if (!ambient_trails_.empty()) ambient_trails_[i].push_back(way.ambient);
    }
    ways_.clear();
    for (const auto& way : item_ways_) mutable_item_trail(way.id).push_back(way);
    item_ways_.clear();
    const double screen = screen_.load(std::memory_order_relaxed);
    for (std::size_t i = 0; i < trails_.size(); ++i) {
        auto& trail = trails_[i];
        std::size_t gone = 0;
        while (gone + 1 < trail.size() &&
               static_cast<double>(change_trails_.empty() ? trail[gone + 1].start : change_trails_[i][gone + 1]) <=
                   screen) {
            ++gone;
        }
        trail.drop_prefix(gone);
        if (!life_trails_.empty()) {
            auto& life_trail = life_trails_[i];
            life_trail.drop_prefix(gone);
            auto& dream_trail = dream_trails_[i];
            dream_trail.drop_prefix(gone);
            auto& changes = change_trails_[i];
            changes.drop_prefix(gone);
            auto& works = work_trails_[i];
            works.drop_prefix(gone);
            auto& thermal = thermal_trails_[i];
            thermal.drop_prefix(gone);
            auto& ambient = ambient_trails_[i];
            ambient.drop_prefix(gone);
            auto& know = knowledge_trails_[i];
            know.drop_prefix(gone);
        }
    }
    for (auto& [id, trail] : item_trails_) {
        std::size_t gone = 0;
        while (gone + 1 < trail->size() && static_cast<double>((*trail)[gone + 1].key.second) <= screen) ++gone;
        if (gone) {
            auto& rows = mutable_item_trail(id);
            rows.erase(rows.begin(), rows.begin() + static_cast<std::ptrdiff_t>(gone));
        }
    }
    std::erase_if(item_trails_, [&](const auto& entry) {
        const auto* archived = crowd_.world().archived_item(entry.first);
        return archived && static_cast<double>(archived->archived_at) <= screen;
    });
    fill(snapshots_.back());
    snapshots_.publish();

    const double seconds = spent();
    if (seconds > 0.0) {
        const double measured = static_cast<double>(reached - frontier) / seconds;
        capacity_.store(can > 0.0 ? can * 0.8 + measured * 0.2 : measured, std::memory_order_relaxed);
    }
    events_.fetch_add(crowd_.world().events_run() - before, std::memory_order_relaxed);
    batches_.fetch_add(1, std::memory_order_relaxed);
    last_batch_ms_.store(seconds * 1000.0, std::memory_order_relaxed);
    if (keeper_ != nullptr) {
        keeper_->history(history_);
    }
    if (!history_.empty()) {
        std::lock_guard lock(greetings_mutex_);
        for (const world::Record& r : history_) {
            if (r.what == static_cast<std::uint32_t>(demo::Happened::greeting)) {
                greetings_.push_back({r.key.second, r.a, r.b});
                greetings_seen_.fetch_add(1, std::memory_order_relaxed);
            }
        }
        history_.clear();
    }
    return reached;
}

std::vector<Greeting> CrowdStepper::drain_greetings() {
    std::lock_guard lock(greetings_mutex_);
    std::vector<Greeting> out;
    out.swap(greetings_);
    return out;
}

void CrowdStepper::pin(std::vector<int> cores) {
    std::lock_guard lock(pin_mutex_);
    pin_ = std::move(cores);
    pin_asked_ = true;
}

}  // namespace kd::view
