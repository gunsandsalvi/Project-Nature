#include "crowd_core.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <utility>

#if defined(__linux__) || defined(__ANDROID__)
#include <sched.h>
#endif

#include "kd/demo/parts.hpp"

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

const world::Activity& Snapshot::way_at(std::size_t i, double t) const {
    std::uint32_t k = first[i];
    while (k + 1 < first[i + 1] && static_cast<double>(ways[k + 1].start) <= t) {
        ++k;
    }
    return ways[k];
}

CrowdStepper::CrowdStepper(demo::CrowdWorld& crowd) : crowd_(crowd) {
    const world::World& w = crowd_.world();
    const auto& raw = w.beings().raw();
    w.beings().each([&](ecs::Id id, world::Beings::Handle h) {
        if (id.family() == ecs::Family::place) {
            camp_ids_.push_back(id);
            camp_places_.push_back(raw.get<world::Place>(h).at);
        }
    });
    const std::vector<ecs::Id>& camps = camp_ids_;
    w.beings().each([&](ecs::Id id, world::Beings::Handle h) {
        if (id.family() != ecs::Family::marker) {
            return;
        }
        const ecs::Id camp = raw.get<demo::Home>(h).camp;
        const auto c = static_cast<std::uint32_t>(std::lower_bound(camps.begin(), camps.end(), camp) - camps.begin());
        ids_.push_back(id);
        walkers_.push_back({id.value, raw.get<demo::MarkerKind>(h).kind, c});
        trails_.push_back({raw.get<world::Activity>(h)});
    });
    crowd_.world().keep_history(&history_);
    crowd_.world().keep_ways(&ways_);
    // the first snapshot, so the screen has something to draw before the first batch
    fill(snapshots_.back());
    snapshots_.publish();
}

void CrowdStepper::fill(Snapshot& s) const {
    s.frontier = crowd_.world().frontier();
    s.walkers = walkers_;
    s.ways.clear();
    s.first.clear();
    for (const std::vector<world::Activity>& trail : trails_) {
        s.first.push_back(static_cast<std::uint32_t>(s.ways.size()));
        s.ways.insert(s.ways.end(), trail.begin(), trail.end());
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
    }
    ways_.clear();
    const double screen = screen_.load(std::memory_order_relaxed);
    for (std::vector<world::Activity>& trail : trails_) {
        std::size_t gone = 0;
        while (gone + 1 < trail.size() && static_cast<double>(trail[gone + 1].start) <= screen) {
            ++gone;
        }
        trail.erase(trail.begin(), trail.begin() + static_cast<std::ptrdiff_t>(gone));
    }
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
