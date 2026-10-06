#include "kd/world/world.hpp"

#include <algorithm>

#include "kd/chance/chance.hpp"
#include "kd/num/sort.hpp"
#include "kd/num/whole.hpp"

namespace kd::world {

static_assert(World::kTorus.width() % World::kIslandCell == 0 && World::kTorus.height() % World::kIslandCell == 0,
              "the islands' cells divide the world's sides");

namespace {

// Ids sorted, each once.
void sort_unique(std::vector<ecs::Id>& ids) {
    std::stable_sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
}

// Joins owners into islands: a union-find whose roots are always the smaller index, so the result does not depend
// on the order of the unions.
class Unions {
public:
    explicit Unions(std::size_t n) : parent_(n) {
        for (std::size_t i = 0; i < n; ++i) {
            parent_[i] = static_cast<std::uint32_t>(i);
        }
    }

    std::uint32_t find(std::uint32_t i) {
        while (parent_[i] != i) {
            parent_[i] = parent_[parent_[i]];
            i = parent_[i];
        }
        return i;
    }

    void join(std::uint32_t a, std::uint32_t b) {
        const std::uint32_t ra = find(a);
        const std::uint32_t rb = find(b);
        if (ra != rb) {
            parent_[std::max(ra, rb)] = std::min(ra, rb);
        }
    }

private:
    std::vector<std::uint32_t> parent_;
};

}  // namespace

// --- Context

bool Context::holds(ecs::Id id) const {
    if (island_ == nullptr) {
        return true;
    }
    const std::optional<std::uint32_t> i = w_.island_of(id);
    return i.has_value() && *i == island_->index;
}

void Context::touch(ecs::Id other) const {
    KD_CHECK(holds(other), "world::Context: a rule touched an owner of another island (A3.3)");
}

std::span<const ecs::Id> Context::moved() const {
    return island_ != nullptr ? std::span<const ecs::Id>(island_->moved) : std::span<const ecs::Id>();
}

void Context::moved(ecs::Id id) {
    if (island_ != nullptr) {
        island_->moved.push_back(id);
    }
}

void Context::schedule(ecs::Id owner, std::uint32_t slot, time::Seconds at) {
    KD_CHECK(slot < Schedule::kSlots, "world::Context: no such slot");
    KD_CHECK(holds(owner), "world::Context: a rule scheduled for an owner of another island (A3.3)");
    Schedule* s = w_.schedule_of(owner);
    KD_CHECK(s != nullptr, "world::Context: no owner with that id to schedule for");
    const event::Key key{at, owner.value, s->next};
    if (in_event_) {
        KD_CHECK(current_ < key, "world::Context: a handler scheduled an event at or before its own");
        KD_CHECK(key.second > current_.second || key.owner == current_.owner,
                 "world::Context: an effect on another owner lands a second later at least (A3.3)");
    } else {
        KD_CHECK(at >= w_.frontier_, "world::Context: an event scheduled before the frontier");
    }
    ++s->next;
    const bool replaced = s->expected[slot] != 0;
    s->expected[slot] = key.sequence;
    const event::Event e{key, slot};
    if (island_ != nullptr) {
        island_->replaced += replaced ? 1 : 0;
        if (at < island_->end) {
            island_->queue.push(e);
        } else {
            island_->outgoing.push_back(e);
        }
        return;
    }
    if (owner.family() == ecs::Family::none) {
        w_.owner_due_[owner.value][slot] = at;
    }
    if (replaced) {
        w_.died();
    }
    w_.queue_.push(e);
}

void Context::cancel(ecs::Id owner, std::uint32_t slot) {
    KD_CHECK(holds(owner), "world::Context: a rule cancelled for an owner of another island (A3.3)");
    Schedule* s = w_.schedule_of(owner);
    KD_CHECK(s != nullptr && slot < Schedule::kSlots, "world::Context: no such owner or slot to cancel");
    if (s->expected[slot] == 0) {
        return;
    }
    s->expected[slot] = 0;
    if (island_ != nullptr) {
        ++island_->replaced;
        return;
    }
    w_.died();
}

void Context::record(std::uint32_t what, std::uint64_t a, std::uint64_t b) {
    KD_CHECK(in_event_, "world::Context: history is recorded at an event");
    const Record r{current_, records_++, what, a, b};
    if (island_ != nullptr) {
        island_->history.push_back(r);
    } else {
        w_.add_history(r);
    }
}

void Context::run(const event::Event& e) {
    const ecs::Id owner{e.key.owner};
    w_.schedule_of(owner)->expected[e.slot] = 0;
    now_ = e.key.second;
    current_ = e.key;
    records_ = 0;
    in_event_ = true;
    w_.system_of(owner).handle(*this, e);
    in_event_ = false;
}

// --- World

World::World(std::uint64_t seed, const data::Catalogue& catalogue) : seed_(seed), catalogue_(catalogue) {}

Beings::Handle World::make_being(ecs::Family f) {
    KD_CHECK(!in_islands_, "world::World: beings are made only between windows");
    const ecs::Id id = ids_.make(f);
    const Beings::Handle h = beings_.make(id);
    beings_.raw().emplace<Schedule>(h);
    return h;
}

void World::end_being(ecs::Id id) {
    KD_CHECK(!in_islands_, "world::World: beings end only between windows");
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

void World::schedule(ecs::Id owner, std::uint32_t slot, time::Seconds at) {
    KD_CHECK(!context_.in_event_ && !in_islands_, "world::World: a handler schedules through its context");
    context_.schedule(owner, slot, at);
}

void World::cancel(ecs::Id owner, std::uint32_t slot) {
    KD_CHECK(!context_.in_event_ && !in_islands_, "world::World: a handler cancels through its context");
    context_.cancel(owner, slot);
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

std::optional<std::uint32_t> World::island_of(ecs::Id id) const {
    const auto at = std::lower_bound(window_ids_.begin(), window_ids_.end(), id);
    if (at == window_ids_.end() || *at != id) {
        return std::nullopt;
    }
    return window_island_[static_cast<std::size_t>(at - window_ids_.begin())];
}

time::Seconds World::next_layer_event() const {
    time::Seconds next = INT64_MAX;
    for (std::size_t o = 0; o < owners_.size(); ++o) {
        for (std::size_t slot = 0; slot < Schedule::kSlots; ++slot) {
            if (owners_[o].expected[slot] != 0) {
                next = std::min(next, owner_due_[o][slot]);
            }
        }
    }
    return next;
}

void World::add_history(const Record& r) {
    num::Digest d;
    d.u64(history_hash_);
    d.i64(r.key.second);
    d.u64(r.key.owner);
    d.u64(r.key.sequence);
    d.u32(r.n);
    d.u32(r.what);
    d.u64(r.a);
    d.u64(r.b);
    history_hash_ = d.value();
    ++history_count_;
    if (history_list_ != nullptr) {
        history_list_->push_back(r);
    }
}

void World::died() {
    queue_.died([this](const event::Event& e) { return live(e); });
}

void World::run_to(time::Seconds goal) {
    KD_CHECK(goal >= frontier_, "world::World: the goal is behind the frontier");
    while (!queue_.empty() && queue_.top().key.second < goal) {
        const event::Event e = queue_.pop();
        if (!live(e)) {
            queue_.skipped();
            continue;
        }
        context_.run(e);
        ++events_;
    }
    frontier_ = goal;
    context_.now_ = goal;
}

void World::run_islands(time::Seconds goal, run::Workers& workers, time::Seconds window) {
    KD_CHECK(goal >= frontier_ && window >= 1, "world::World: islands need a goal ahead and a window");
    while (frontier_ < goal) {
        const time::Seconds a = frontier_;
        // the world's own owners come first within their second, and run alone, since a layer may touch anything
        while (!queue_.empty() && queue_.top().key.second == a && queue_.top().key.owner < ecs::owners::count) {
            const event::Event e = queue_.pop();
            if (!live(e)) {
                queue_.skipped();
                continue;
            }
            context_.run(e);
            ++events_;
        }
        // windows on a fixed grid of game time, also cut at the goal and at the next layer's event
        const time::Seconds b = std::min({goal, (num::floor_div(a, window) + 1) * window, next_layer_event()});
        KD_CHECK(b > a, "world::World: a window must hold some time");
        run_window(a, b, workers);
        frontier_ = b;
        context_.now_ = b;
    }
}

void World::run_window(time::Seconds a, time::Seconds b, run::Workers& workers) {
    std::vector<event::Event> events;
    while (!queue_.empty() && queue_.top().key.second < b) {
        const event::Event e = queue_.pop();
        if (!live(e)) {
            queue_.skipped();
            continue;
        }
        KD_CHECK(e.key.owner >= ecs::owners::count, "world::World: a layer's event inside a window");
        events.push_back(e);
    }
    if (events.empty()) {
        return;
    }
    std::vector<ecs::Id> active;
    active.reserve(events.size());
    for (const event::Event& e : events) {
        active.push_back(ecs::Id{e.key.owner});
    }
    sort_unique(active);
    const std::size_t count = form_islands(a, b, active);

    // the islands that have events, each with its own queue
    std::vector<std::int64_t> slot(count, -1);
    std::vector<Island> islands;
    for (const event::Event& e : events) {
        const std::uint32_t i = *island_of(ecs::Id{e.key.owner});
        if (slot[i] < 0) {
            slot[i] = static_cast<std::int64_t>(islands.size());
            Island is;
            is.index = i;
            is.end = b;
            islands.push_back(std::move(is));
        }
        islands[static_cast<std::size_t>(slot[i])].queue.push(e);
    }
    in_islands_ = true;
    workers.for_each(islands.size(), [&](std::size_t k) {
        Island& is = islands[k];
        Context c(*this, &is);
        while (!is.queue.empty()) {
            const event::Event e = is.queue.pop();
            if (!live(e)) {
                continue;
            }
            c.run(e);
            ++is.events;
        }
    });
    in_islands_ = false;

    // their new events, counts and moved owners, merged in the islands' order; the queue's order is the keys'
    std::vector<ecs::Id> moved;
    std::vector<Record> history;
    std::size_t replaced = 0;
    for (Island& is : islands) {
        history.insert(history.end(), is.history.begin(), is.history.end());
        for (const event::Event& e : is.outgoing) {
            if (live(e)) {
                queue_.push(e);
            }
        }
        replaced += is.replaced;
        events_ += is.events;
        moved.insert(moved.end(), is.moved.begin(), is.moved.end());
    }
    queue_.died([this](const event::Event& e) { return live(e); }, replaced);
    num::sort_strict(history.begin(), history.end(),
                     [](const Record& x, const Record& y) { return x.key != y.key ? x.key < y.key : x.n < y.n; });
    for (const Record& r : history) {
        add_history(r);
    }
    sort_unique(moved);
    std::vector<System*> systems;
    for (System* s : by_family_) {
        if (s != nullptr && std::find(systems.begin(), systems.end(), s) == systems.end()) {
            systems.push_back(s);
        }
    }
    num::sort_strict(systems.begin(), systems.end(),
                     [](const System* x, const System* y) { return x->name() < y->name(); });
    for (System* s : systems) {
        s->after_window(*this, moved);
    }
    islands_run_ = islands.size();
    std::uint64_t most = 0;
    std::uint64_t all = 0;
    for (const Island& is : islands) {
        most = std::max(most, is.events);
        all += is.events;
    }
    ++counts_.windows;
    counts_.islands += islands.size();
    counts_.owners += window_ids_.size();
    counts_.events += all;
    counts_.largest_events += most;
    std::vector<std::uint64_t> sizes(count, 0);
    for (const std::uint32_t i : window_island_) {
        ++sizes[i];
    }
    largest_island_ = sizes.empty() ? 0 : *std::max_element(sizes.begin(), sizes.end());
    window_ids_.clear();
    window_island_.clear();
}

std::size_t World::form_islands(time::Seconds a, time::Seconds b, std::span<const ecs::Id> active) {
    // each active owner's bound, from its system, and the farthest any two owners touch from
    std::vector<System*> systems;
    for (const ecs::Id id : active) {
        System* s = &system_of(id);
        if (std::find(systems.begin(), systems.end(), s) == systems.end()) {
            systems.push_back(s);
        }
    }
    std::vector<Bound> bounds;
    std::int64_t reach = 0;
    for (System* s : systems) {
        std::vector<ecs::Id> mine;
        for (const ecs::Id id : active) {
            if (&system_of(id) == s) {
                mine.push_back(id);
            }
        }
        s->bounds(*this, a, b, mine, bounds);
        reach = std::max(reach, s->reach());
    }
    KD_CHECK(bounds.size() == active.size(), "world::World: a system gave no bound for an owner with events");
    num::sort_strict(bounds.begin(), bounds.end(), [](const Bound& x, const Bound& y) { return x.id < y.id; });

    // the owners with no events near each bound join it
    std::vector<ecs::Id> nodes(active.begin(), active.end());
    std::vector<std::pair<ecs::Id, ecs::Id>> links;
    std::vector<ecs::Id> near;
    for (const Bound& bound : bounds) {
        near.clear();
        system_of(bound.id).near(*this, a, b, bound, near);
        for (const ecs::Id id : near) {
            nodes.push_back(id);
            links.emplace_back(bound.id, id);
        }
    }
    sort_unique(nodes);
    const auto index = [&](ecs::Id id) {
        return static_cast<std::uint32_t>(std::lower_bound(nodes.begin(), nodes.end(), id) - nodes.begin());
    };
    Unions unions(nodes.size());
    for (const auto& [x, y] : links) {
        unions.join(index(x), index(y));
    }

    // active owners whose circles come within reach of each other, found cell by cell
    const std::int64_t nx = torus_.width() / kIslandCell;
    const std::int64_t ny = torus_.height() / kIslandCell;
    struct InCell {
        std::int64_t cell;
        std::uint32_t bound;
    };
    std::vector<InCell> cells;
    for (std::size_t i = 0; i < bounds.size(); ++i) {
        const Bound& bd = bounds[i];
        const std::int64_t r = bd.radius + reach / 2 + 1;
        const std::int64_t kx0 = num::floor_div(bd.centre.x - r, kIslandCell);
        const std::int64_t kx1 = std::min(num::floor_div(bd.centre.x + r, kIslandCell), kx0 + nx - 1);
        const std::int64_t ky0 = num::floor_div(bd.centre.y - r, kIslandCell);
        const std::int64_t ky1 = std::min(num::floor_div(bd.centre.y + r, kIslandCell), ky0 + ny - 1);
        for (std::int64_t ky = ky0; ky <= ky1; ++ky) {
            for (std::int64_t kx = kx0; kx <= kx1; ++kx) {
                cells.push_back({num::floor_mod(ky, ny) * nx + num::floor_mod(kx, nx), static_cast<std::uint32_t>(i)});
            }
        }
    }
    num::sort_strict(cells.begin(), cells.end(), [&](const InCell& x, const InCell& y) {
        return x.cell != y.cell ? x.cell < y.cell : bounds[x.bound].id < bounds[y.bound].id;
    });
    for (std::size_t first = 0; first < cells.size();) {
        std::size_t last = first;
        while (last < cells.size() && cells[last].cell == cells[first].cell) {
            ++last;
        }
        for (std::size_t p = first; p < last; ++p) {
            for (std::size_t q = p + 1; q < last; ++q) {
                const Bound& x = bounds[cells[p].bound];
                const Bound& y = bounds[cells[q].bound];
                const std::int64_t within = x.radius + y.radius + reach;
                if (within >= (std::int64_t{1} << 31) ||
                    torus_.squared_distance(x.centre, y.centre) <= within * within) {
                    unions.join(index(x.id), index(y.id));
                }
            }
        }
        first = last;
    }

    // the islands, numbered in the order of their smallest owner
    window_ids_ = nodes;
    window_island_.assign(nodes.size(), 0);
    std::vector<std::int64_t> label(nodes.size(), -1);
    std::uint32_t count = 0;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        const std::uint32_t root = unions.find(static_cast<std::uint32_t>(i));
        if (label[root] < 0) {
            label[root] = count++;
        }
        window_island_[i] = static_cast<std::uint32_t>(label[root]);
    }
    return count;
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
    if (workers_ != nullptr && until - frontier_ > window_) {
        run_islands(until, *workers_, window_);
    } else {
        run_to(until);
    }
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
                         [](const System* x, const System* y) { return x->name() < y->name(); });
        num::Digest d;
        for (const System* s : systems) {
            d.text(s->name());
            s->digest(d);
        }
        out.systems = d.value();
    }
    {
        num::Digest d;
        d.u64(history_count_);
        d.u64(history_hash_);
        out.history = d.value();
    }
    num::Digest whole;
    for (const std::uint64_t part : {out.clock, out.queue, out.beings, out.things, out.systems, out.history}) {
        whole.u64(part);
    }
    out.whole = whole.value();
    return out;
}

}  // namespace kd::world
