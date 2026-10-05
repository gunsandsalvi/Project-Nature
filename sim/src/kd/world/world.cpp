#include "kd/world/world.hpp"

#include <algorithm>

#include "kd/chance/chance.hpp"
#include "kd/num/sort.hpp"

namespace kd::world {

World::World(std::uint64_t seed, const data::Catalogue& catalogue) : seed_(seed), catalogue_(catalogue) {}

Beings::Handle World::make_being(ecs::Family f) {
    const ecs::Id id = ids_.make(f);
    const Beings::Handle h = beings_.make(id);
    beings_.raw().emplace<Schedule>(h);
    return h;
}

void World::end_being(ecs::Id id) {
    Schedule* s = schedule_of(id);
    KD_CHECK(s != nullptr, "world::World: no being with that id to end");
    for (std::uint64_t& expected : s->expected) {
        if (expected != 0) {
            expected = 0;
            died();
        }
    }
    beings_.end(id);
}

void World::set_system(ecs::Family f, System& s) {
    by_family_[static_cast<std::size_t>(f)] = &s;
}

void World::set_layer(ecs::Id owner, System& s) {
    KD_CHECK(owner.family() == ecs::Family::none && owner.value < ecs::owners::count,
             "world::World: a layer is one of the world's own owners");
    by_owner_[owner.value] = &s;
}

Schedule* World::schedule_of(ecs::Id owner) {
    if (owner.family() == ecs::Family::none) {
        return owner.value < owners_.size() ? &owners_[owner.value] : nullptr;
    }
    const std::optional<Beings::Handle> h = beings_.find(owner);
    return h ? beings_.raw().try_get<Schedule>(*h) : nullptr;
}

const Schedule* World::schedule_of(ecs::Id owner) const {
    if (owner.family() == ecs::Family::none) {
        return owner.value < owners_.size() ? &owners_[owner.value] : nullptr;
    }
    const std::optional<Beings::Handle> h = beings_.find(owner);
    return h ? beings_.raw().try_get<Schedule>(*h) : nullptr;
}

bool World::live(const event::Event& e) const {
    const Schedule* s = schedule_of(ecs::Id{e.key.owner});
    return s != nullptr && s->expected[e.slot] == e.key.sequence;
}

System& World::system_of(ecs::Id owner) {
    System* s = owner.family() == ecs::Family::none ? by_owner_[owner.value]
                                                    : by_family_[static_cast<std::size_t>(owner.family())];
    KD_CHECK(s != nullptr, "world::World: no system handles this owner's events");
    return *s;
}

void World::died() {
    queue_.died([this](const event::Event& e) { return live(e); });
}

void World::schedule(ecs::Id owner, std::uint32_t slot, time::Seconds at) {
    KD_CHECK(slot < Schedule::kSlots, "world::World: no such slot");
    Schedule* s = schedule_of(owner);
    KD_CHECK(s != nullptr, "world::World: no owner with that id to schedule for");
    const event::Key key{at, owner.value, s->next};
    if (in_event_) {
        KD_CHECK(current_ < key, "world::World: a handler scheduled an event at or before its own");
        KD_CHECK(key.second > current_.second || key.owner == current_.owner,
                 "world::World: an effect on another owner lands a second later at least (A3.3)");
    } else {
        KD_CHECK(at >= frontier_, "world::World: an event scheduled before the frontier");
    }
    ++s->next;
    const bool replaced = s->expected[slot] != 0;
    s->expected[slot] = key.sequence;
    if (replaced) {
        died();
    }
    queue_.push({key, slot});
}

void World::cancel(ecs::Id owner, std::uint32_t slot) {
    Schedule* s = schedule_of(owner);
    KD_CHECK(s != nullptr && slot < Schedule::kSlots, "world::World: no such owner or slot to cancel");
    if (s->expected[slot] != 0) {
        s->expected[slot] = 0;
        died();
    }
}

void World::run_to(time::Seconds goal) {
    KD_CHECK(goal >= frontier_, "world::World: the goal is behind the frontier");
    while (!queue_.empty() && queue_.top().key.second < goal) {
        const event::Event e = queue_.pop();
        Schedule* s = schedule_of(ecs::Id{e.key.owner});
        if (s == nullptr || s->expected[e.slot] != e.key.sequence) {
            queue_.skipped();
            continue;
        }
        s->expected[e.slot] = 0;
        now_ = e.key.second;
        current_ = e.key;
        in_event_ = true;
        system_of(ecs::Id{e.key.owner}).handle(*this, e);
        in_event_ = false;
        ++events_;
    }
    frontier_ = goal;
    now_ = goal;
}

time::Seconds World::advance(time::Seconds frontier, time::Seconds goal) {
    KD_CHECK(frontier == frontier_, "world::World: the runner's frontier is not the world's");
    const time::Seconds until = std::min(goal, frontier_ + time::kDay);
    if (fuzz_) {
        const chance::Draws draws(*fuzz_, chance::name("order fuzzer"), 0, 0, chance::name("batch"));
        const std::uint64_t key = draws.bits(batches_);
        beings_.fuzz(key);
        things_.fuzz(key);
    }
    ++batches_;
    run_to(until);
    return until;
}

Digests World::digests() const {
    Digests out;
    {
        num::Digest d;
        d.i64(frontier_);
        d.u64(ids_.next());
        for (const Schedule& s : owners_) {
            ecs::digest_component(s, d);
        }
        out.clock = d.value();
    }
    {
        num::Digest d;
        const std::vector<event::Event> events =
            queue_.live_in_order([this](const event::Event& e) { return live(e); });
        d.u64(events.size());
        for (const event::Event& e : events) {
            d.i64(e.key.second);
            d.u64(e.key.owner);
            d.u64(e.key.sequence);
            d.u32(e.slot);
        }
        out.queue = d.value();
    }
    {
        num::Digest d;
        beings_.digest(d);
        out.beings = d.value();
    }
    {
        num::Digest d;
        things_.digest(d);
        out.things = d.value();
    }
    {
        // the systems in the order of their names
        std::vector<const System*> systems;
        for (const System* s : by_family_) {
            if (s != nullptr && std::find(systems.begin(), systems.end(), s) == systems.end()) {
                systems.push_back(s);
            }
        }
        for (const System* s : by_owner_) {
            if (s != nullptr && std::find(systems.begin(), systems.end(), s) == systems.end()) {
                systems.push_back(s);
            }
        }
        num::sort_strict(systems.begin(), systems.end(),
                         [](const System* a, const System* b) { return a->name() < b->name(); });
        num::Digest d;
        for (const System* s : systems) {
            d.text(s->name());
            s->digest(d);
        }
        out.systems = d.value();
    }
    num::Digest whole;
    for (const std::uint64_t part : {out.clock, out.queue, out.beings, out.things, out.systems}) {
        whole.u64(part);
    }
    out.whole = whole.value();
    return out;
}

}  // namespace kd::world
