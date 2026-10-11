#include "kd/world/world.hpp"
#include "kd/world/craft_store.hpp"
#include "kd/world/fire_store.hpp"

#include <algorithm>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>

#include "kd/chance/chance.hpp"
#include "kd/data/craft.hpp"
#include "kd/demo/fire.hpp"
#include "kd/num/sort.hpp"
#include "kd/num/whole.hpp"
#include "kd/world/walk.hpp"

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
    if (w_.item_event_) {
        const auto h = w_.beings_.find(id);
        const auto* work = h ? std::as_const(w_.beings_).raw().try_get<Work>(*h) : nullptr;
        const auto listed = std::lower_bound(w_.makers_.begin(), w_.makers_.end(), id,
                                             [](const auto& entry, auto wanted) { return entry.id < wanted; });
        if ((work && work->state == 2) || (listed != w_.makers_.end() && listed->id == id)) w_.makers_changed();
    }
    if (island_ != nullptr) {
        island_->moved.push_back(id);
    }
    if (w_.ways_list_ == nullptr) {
        return;
    }
    KD_CHECK(in_event_, "world::Context: a way begins at an event");
    const Activity* a = w_.beings_.raw().try_get<Activity>(w_.beings_.handle(id));
    if (a == nullptr) {
        return;
    }
    const auto* life = w_.beings_.raw().try_get<Life>(w_.beings_.handle(id));
    const auto* dream = w_.beings_.raw().try_get<Dream>(w_.beings_.handle(id));
    const auto* work = w_.beings_.raw().try_get<Work>(w_.beings_.handle(id));
    const auto* know = w_.beings_.raw().try_get<Knowledge>(w_.beings_.handle(id));
    const auto* thermal = w_.beings_.raw().try_get<Thermal>(w_.beings_.handle(id));
    const auto* home = w_.beings_.raw().try_get<demo::Home>(w_.beings_.handle(id));
    const auto* ambient = home ? w_.beings_.raw().try_get<Ambient>(w_.beings_.handle(home->camp)) : nullptr;
    KnowledgeView knowledge;
    if (know) {
        auto& previous = knowledge_views_[id];
        previous = KnowledgeView::capture(*know, previous);
        knowledge = previous;
    }
    const Way way{current_,
                  ways_++,
                  id,
                  *a,
                  life ? std::optional<Life>(*life) : std::nullopt,
                  dream ? std::optional<Dream>(*dream) : std::nullopt,
                  work ? std::optional<Work>(*work) : std::nullopt,
                  std::move(knowledge),
                  thermal ? std::optional<Thermal>(*thermal) : std::nullopt,
                  ambient ? std::optional<Ambient>(*ambient) : std::nullopt};
    if (island_ != nullptr) {
        island_->ways.push_back(way);
    } else {
        w_.ways_list_->push_back(way);
    }
}

void Context::item_changed(ecs::Id id) {
    w_.physical_changed(id);
    if (!notifying_item_) {
        const auto present = w_.things_.find(id);
        const auto* physical = present ? w_.things_.raw().try_get<Item>(*present) : nullptr;
        if (physical && physical->home.value) {
            notifying_item_ = true;
            w_.system_of(physical->home).item_changed(*this, id);
            notifying_item_ = false;
        }
    }
    w_.item_site_dirty_.push_back(id);
    if (!w_.item_ways_list_) return;
    KD_CHECK(in_event_ && !island_, "Craft item snapshots are emitted in reference event order");
    const auto h = w_.things_.handle(id);
    const auto* fire = w_.things_.raw().try_get<Fire>(h);
    const auto* timer = w_.things_.raw().try_get<HeatTimer>(h);
    w_.item_ways_list_->push_back({current_, ways_++, id, w_.things_.raw().get<Place>(h), w_.things_.raw().get<Item>(h),
                                   fire ? std::optional<Fire>(*fire) : std::nullopt,
                                   timer ? std::optional<HeatTimer>(*timer) : std::nullopt});
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
    const auto family = ecs::Id{e.key.owner}.family();
    const auto measured = w_.measure(family == ecs::Family::person  ? Cost::people
                                     : family == ecs::Family::place ? Cost::camp
                                                                    : Cost::layers);
    const ecs::Id owner{e.key.owner};
    w_.schedule_of(owner)->expected[e.slot] = 0;
    now_ = e.key.second;
    current_ = e.key;
    records_ = 0;
    ways_ = 0;
    in_event_ = true;
    const bool item_event = !std::as_const(w_.beings_).raw().view<Work>().empty();
    if (item_event) {
        w_.item_event_ = true;  // Craft windows use reference event order.
        w_.makers_changed();
    }
    w_.system_of(owner).handle(*this, e);
    if (item_event) w_.item_event_ = false;
    in_event_ = false;
}

// --- Commands

void Commands::handle(Context& c, const event::Event& /*e*/) {
    KD_CHECK(taker_ != nullptr, "world::Commands: no system takes commands");
    std::size_t done = 0;
    while (done < pending_.size() && pending_[done].at == c.now()) {
        const Command& cmd = pending_[done];
        c.record(kActed, cmd.number, cmd.a);
        taker_->command(c, cmd);
        ++done;
    }
    pending_.erase(pending_.begin(), pending_.begin() + static_cast<std::ptrdiff_t>(done));
    if (!pending_.empty()) {
        c.schedule(ecs::owners::commands, kActivitySlot, pending_.front().at);
    }
}

void Commands::wake() {
    // the layer's one event at the earliest pending command's second, made again only when that second changes
    const time::Seconds first = pending_.front().at;
    const std::uint64_t owner = ecs::owners::commands.value;
    if (w_.owners_[owner].expected[kActivitySlot] == 0 || w_.owner_due_[owner][kActivitySlot] != first) {
        w_.schedule(ecs::owners::commands, kActivitySlot, first);
    }
}

void Commands::digest(num::Digest& d) const {
    d.u64(made_);
    d.u64(pending_.size());
    for (const Command& c : pending_) {
        d.u64(c.number);
        d.i64(c.at);
        d.u32(c.what);
        d.u64(c.a);
        d.u64(c.b);
    }
}

void Commands::save(ByteWriter& w) const {
    w.u64(made_);
    w.u64(pending_.size());
    for (const Command& c : pending_) {
        w.u64(c.number);
        w.i64(c.at);
        w.u32(c.what);
        w.u64(c.a);
        w.u64(c.b);
    }
}

bool Commands::load(ByteReader& r) {
    std::uint64_t n = 0;
    if (!r.u64(made_) || !r.u64(n)) {
        return false;
    }
    pending_.clear();
    for (std::uint64_t i = 0; i < n; ++i) {
        Command c;
        if (!r.u64(c.number) || !r.i64(c.at) || !r.u32(c.what) || !r.u64(c.a) || !r.u64(c.b)) {
            return false;
        }
        pending_.push_back(c);
    }
    return r.finished();
}

// --- World

World::World(std::uint64_t seed, const data::Catalogue& catalogue) : seed_(seed), catalogue_(catalogue) {
    beings_.raw().storage<Camp>();
    beings_.raw().storage<Person>();
    beings_.raw().storage<Life>();
    beings_.raw().storage<Habitat>();
    beings_.raw().storage<Work>();
    beings_.raw().storage<Knowledge>();
    beings_.raw().storage<CraftHistory>();
    beings_.raw().storage<Lessons>();
    things_.raw().storage<Item>();
    things_.raw().storage<Fire>();
    things_.raw().storage<HeatTimer>();
    beings_.raw().storage<Thermal>();
    beings_.raw().storage<Ambient>();
    set_layer(ecs::owners::commands, commands_);
}

const std::vector<World::ItemSite>& World::item_sites() const {
    std::vector<std::size_t> changed;
    const auto own = [&](ecs::Id id, std::optional<Things::Handle> h) {
        const auto* item = h ? things_.raw().try_get<Item>(*h) : nullptr;
        const bool tracked = item && item->owner.value &&
                             (item->mass > 0 || things_.raw().all_of<HeatTimer>(h.value_or(Things::Handle{})));
        const auto key =
            item ? std::pair{item->owner.value, item->kind} : std::pair{std::uint64_t{0}, std::uint32_t{0}};
        const auto old = item_owner_of_.find(id.value);
        if (old != item_owner_of_.end()) {
            if (tracked && key == old->second) return;
            auto group = owned_items_.find(old->second);
            KD_CHECK(group != owned_items_.end(), "An indexed owner has its item group");
            std::erase_if(group->second, [&](const auto& e) { return e.id == id; });
            if (group->second.empty()) owned_items_.erase(group);
            item_owner_of_.erase(old);
        }
        if (!tracked) return;
        auto& group = owned_items_[key];
        const auto at =
            std::lower_bound(group.begin(), group.end(), id, [](const auto& e, auto wanted) { return e.id < wanted; });
        group.insert(at, {id, h.value_or(Things::Handle{})});
        item_owner_of_[id.value] = key;
    };
    const auto add = [&](ecs::Id id, Things::Handle h) {
        own(id, h);
        const auto* item = things_.raw().try_get<Item>(h);
        const auto* place = things_.raw().try_get<Place>(h);
        if (!item || !place || item->mass == 0) return;
        const auto [site, fresh] =
            item_site_at_.try_emplace({item->home.value, place->at.x, place->at.y}, item_sites_.size());
        if (fresh) item_sites_.push_back({item->home, place->at, {}, {}, {}});
        item_sites_[site->second].items.push_back({id, h});
        item_site_of_[id.value] = {site->second, item->kind, item->material, item->state, item->owner};
        changed.push_back(site->second);
    };
    if (!item_sites_valid_) {
        item_sites_.clear();
        item_site_at_.clear();
        item_site_of_.clear();
        owned_items_.clear();
        item_owner_of_.clear();
        things_.each(add);
        item_sites_valid_ = true;
    } else {
        sort_unique(item_site_dirty_);
        for (const auto id : item_site_dirty_) {
            const auto old = item_site_of_.find(id.value);
            const auto h = things_.find(id);
            own(id, h);
            if (old != item_site_of_.end()) {
                const auto& address = old->second;
                const auto& site = item_sites_[address.site];
                const auto* item = h ? things_.raw().try_get<Item>(*h) : nullptr;
                const auto* place = h ? things_.raw().try_get<Place>(*h) : nullptr;
                if (item && place && item->mass > 0 && item->home == site.home && place->at == site.at &&
                    item->kind == address.kind && item->material == address.material && item->state == address.state &&
                    item->owner == address.owner)
                    continue;  // Readers fetch current mass, dimensions and properties.
                auto& entries = item_sites_[address.site].items;
                std::erase_if(entries, [&](const auto& entry) { return entry.id == id; });
                changed.push_back(address.site);
                item_site_of_.erase(old);
            }
            if (h) add(id, *h);
        }
    }
    item_site_dirty_.clear();
    std::stable_sort(changed.begin(), changed.end());
    changed.erase(std::unique(changed.begin(), changed.end()), changed.end());
    for (const auto site : changed) {
        auto& group = item_sites_[site];
        num::sort_strict(group.items.begin(), group.items.end(),
                         [](const auto& a, const auto& b) { return a.id < b.id; });
        group.sight.clear();
        group.owned.clear();
        for (const auto& entry : group.items) {
            const auto& item = things_.raw().get<Item>(entry.handle);
            group.owned[item.owner.value].push_back(entry);
            const auto found = std::find_if(group.sight.begin(), group.sight.end(), [&](const auto& seen) {
                return seen.kind == item.kind && seen.material == item.material && seen.state == item.state;
            });
            if (found == group.sight.end())
                group.sight.push_back({item.kind, item.material, item.state, entry.id, entry.id});
            else
                found->last = entry.id;
        }
    }
    // Empty locations are disposable index entries, not physical history.
    // Batch compaction avoids rebuilding all addresses for every moved portion.
    // Between compactions, empty slots stay below 16 or one quarter of slots;
    // the index therefore grows with live sites, never with old positions.
    const bool emptied =
        std::any_of(changed.begin(), changed.end(), [&](auto site) { return item_sites_[site].items.empty(); });
    const auto empty =
        emptied ? static_cast<std::size_t>(std::count_if(item_sites_.begin(), item_sites_.end(),
                                                         [](const auto& site) { return site.items.empty(); }))
                : 0;
    if (empty >= 16 && empty * 4 >= item_sites_.size()) {
        item_site_at_.clear();
        std::size_t kept = 0;
        for (std::size_t site = 0; site < item_sites_.size(); ++site) {
            if (item_sites_[site].items.empty()) continue;
            if (kept != site) item_sites_[kept] = std::move(item_sites_[site]);
            const auto& group = item_sites_[kept];
            item_site_at_.emplace(std::tuple{group.home.value, group.at.x, group.at.y}, kept);
            for (const auto& entry : group.items) {
                const auto found = item_site_of_.find(entry.id.value);
                KD_CHECK(found != item_site_of_.end(), "A live site entry has its indexed address");
                found->second.site = kept;
            }
            ++kept;
        }
        item_sites_.resize(kept);
    }
    return item_sites_;
}

Beings::Handle World::make_being(ecs::Family f) {
    KD_CHECK(!in_islands_, "world::World: beings are made only between windows");
    makers_changed();
    const ecs::Id id = ids_.make(f);
    const Beings::Handle h = beings_.make(id);
    beings_.raw().emplace<Schedule>(h);
    return h;
}

std::span<const World::ItemSite::Entry> World::items_owned(ecs::Id owner, std::uint32_t kind) const {
    (void)item_sites();
    const auto group = owned_items_.find({owner.value, kind});
    return group == owned_items_.end() ? std::span<const ItemSite::Entry>{} : group->second;
}

const World::PhysicalItems& World::physical_items(ecs::Id camp) const {
    const auto describe = [&](Things::Handle h) -> std::optional<PhysicalAddress> {
        const auto* item = things_.raw().try_get<Item>(h);
        if (!item) return {};
        const auto* timer = things_.raw().try_get<HeatTimer>(h);
        const auto* fire = things_.raw().try_get<Fire>(h);
        const std::uint8_t flags = static_cast<std::uint8_t>(
            (item->mass > 0 && (timer || demo::FireRules::cooking_recipe(catalogue_, *item)) ? 1 : 0) |
            (item->mass > 0 && !fire ? 2 : 0) | (timer ? 4 : 0) | (fire ? 8 : 0));
        if (!flags) return {};
        const auto fire_due = fire ? fire->next : 0, food_due = timer ? timer->next : 0;
        return PhysicalAddress{item->home, flags, fire_due, food_due};
    };
    const auto insert = [&](ecs::Id id, Things::Handle h, const PhysicalAddress& address) {
        physical_address_[id] = address;
        auto& group = physical_items_[address.camp];
        for (const auto [bit, entries] :
             {std::pair{1, &group.food}, {2, &group.fuel}, {4, &group.timers}, {8, &group.fires}})
            if (address.flags & bit) {
                const auto at = std::lower_bound(entries->begin(), entries->end(), id,
                                                 [](const auto& entry, auto wanted) { return entry.id < wanted; });
                entries->insert(at, {id, h});
            }
        if (address.fire_due) group.fire_due[address.fire_due].insert(id);
        if (address.food_due) group.food_due[address.food_due].insert(id);
    };
    if (!physical_items_valid_ || scalar_work_) {
        physical_items_.clear();
        physical_address_.clear();
        things_.each([&](ecs::Id id, Things::Handle h) {
            if (const auto address = describe(h)) insert(id, h, *address);
        });
        physical_items_valid_ = true;
    } else {
        sort_unique(physical_dirty_);
        for (const auto id : physical_dirty_) {
            const auto h = things_.find(id);
            const auto desired = h ? describe(*h) : std::optional<PhysicalAddress>{};
            if (const auto at = physical_address_.find(id); at != physical_address_.end()) {
                if (desired && at->second == *desired) continue;
                auto& group = physical_items_[at->second.camp];
                for (auto* entries : {&group.food, &group.fuel, &group.timers, &group.fires})
                    std::erase_if(*entries, [&](const auto& entry) { return entry.id == id; });
                for (const auto [when, deadlines] :
                     {std::pair{at->second.fire_due, &group.fire_due}, {at->second.food_due, &group.food_due}})
                    if (when) {
                        auto bucket = deadlines->find(when);
                        if (bucket != deadlines->end()) {
                            bucket->second.erase(id);
                            if (bucket->second.empty()) deadlines->erase(bucket);
                        }
                    }
                physical_address_.erase(at);
            }
            if (h && desired) insert(id, h.value_or(Things::Handle{}), desired.value_or(PhysicalAddress{}));
        }
    }
    physical_dirty_.clear();
    return physical_items_[camp];
}

const std::vector<World::MakerEntry>& World::makers() const {
    if (!makers_valid_ || scalar_work_) {
        makers_.clear();
        beings_.each([&](ecs::Id id, Beings::Handle h) {
            const auto& raw = beings_.raw();
            if (!raw.all_of<Knowledge, Work, demo::Home>(h)) return;
            const auto& work = raw.get<Work>(h);
            if (work.state == 2 && work.intended && work.try_seconds > 0)
                makers_.push_back({id, raw.get<demo::Home>(h).camp, h});
        });
        makers_valid_ = true;
    }
    return makers_;
}

Things::Handle World::make_thing() {
    KD_CHECK(!in_islands_, "world::World: things are made only between windows");
    const auto id = ids_.make(ecs::Family::thing);
    item_site_dirty_.push_back(id);
    return things_.make(id);
}

void World::end_being(ecs::Id id) {
    context_.knowledge_views_.erase(id);
    makers_changed();
    KD_CHECK(!in_islands_, "world::World: beings end only between windows");
    Schedule* s = schedule_of(id);
    KD_CHECK(s != nullptr, "world::World: no being with that id to end");
    const auto h = beings_.handle(id);
    const auto* person = beings_.raw().try_get<Person>(h);
    const auto* home = beings_.raw().try_get<demo::Home>(h);
    if (person && home) {
        auto* ledger = beings_.raw().try_get<Dreams>(beings_.handle(home->camp));
        if (ledger) {
            const auto at = std::lower_bound(ledger->ended.begin(), ledger->ended.end(), id,
                                             [](const auto& entry, ecs::Id wanted) { return entry.id < wanted; });
            const auto* knowledge = beings_.raw().try_get<Knowledge>(h);
            ledger->ended.insert(at, {id, *person, context_.in_event_ ? context_.now() : frontier_,
                                      knowledge ? knowledge->next_memory : 0});
        }
    }
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

const ArchivedItem* World::archived_item(ecs::Id id) const {
    const auto at = archived_ids_.find(id);
    return at == archived_ids_.end() ? nullptr : &item_archive_[at->second];
}

void World::retain_records(time::Seconds now) {
    const auto measured = measure(Cost::retention);
    std::set<ecs::Id> physical;
    const auto pin = [&](ecs::Id id) {
        if (id.value) physical.insert(id);
    };
    for (const auto h : beings_.raw().view<Work>())
        for (const auto& input : beings_.raw().get<Work>(h).inputs) pin(input.item);
    for (const auto h : beings_.raw().view<Life>()) pin(beings_.raw().get<Life>(h).meal_item);
    for (const auto h : beings_.raw().view<Thermal>()) {
        const auto& thermal = beings_.raw().get<Thermal>(h);
        pin(thermal.tending_input);
    }
    for (const auto h : things_.raw().view<HeatTimer, Item>()) {
        const auto& timer = things_.raw().get<HeatTimer>(h);
        if (things_.raw().get<Item>(h).mass > 0) pin(timer.chance_source);
    }
    for (const auto ch : beings_.raw().view<CraftHistory>()) {
        const auto camp = beings_.id_of(ch);
        auto& history = beings_.raw().get<CraftHistory>(ch);
        // Every validated fact has a nonnegative time. Before the shortest
        // retention window can expire, there is nothing to prune or pin.
        if (!scalar_work_ && now <= 2 * time::kDay) continue;
        std::set<std::uint64_t> choices, events;
        const auto keep_event = [&](std::uint64_t id) {
            if (id) events.insert(id);
        };
        beings_.each([&](ecs::Id, Beings::Handle h) {
            const auto* home = beings_.raw().try_get<demo::Home>(h);
            if (!home || home->camp != camp) return;
            if (const auto* knowledge = beings_.raw().try_get<Knowledge>(h)) {
                if (knowledge->choice) choices.insert(knowledge->choice);
                for (const auto& skill : knowledge->skills) keep_event(skill.source_event);
                const auto evidence = [&](const Familiar& familiar) {
                    for (const auto event : familiar.source_events) keep_event(event);
                };
                for (const auto& familiar : knowledge->familiar) evidence(familiar);
                for (const auto& memory : knowledge->memories) {
                    keep_event(memory.event);
                    for (const auto& input : memory.inputs) evidence(input);
                }
                for (const auto& hunch : knowledge->hunches)
                    for (const auto& input : hunch.inputs) evidence(input);
                for (const auto& peer : knowledge->peers) keep_event(peer.event);
                keep_event(knowledge->last_observed_event);
            }
            if (const auto* work = beings_.raw().try_get<Work>(h); work && work->choice) choices.insert(work->choice);
            if (const auto* thermal = beings_.raw().try_get<Thermal>(h)) {
                if (thermal->warm_choice) choices.insert(thermal->warm_choice);
                if (thermal->tending_choice) choices.insert(thermal->tending_choice);
            }
        });
        things_.each([&](ecs::Id, Things::Handle h) {
            if (const auto* timer = things_.raw().try_get<HeatTimer>(h);
                timer && things_.raw().get<Item>(h).home == camp && timer->placement_choice)
                choices.insert(timer->placement_choice);
        });
        if (const auto* lessons = beings_.raw().try_get<Lessons>(ch))
            for (const auto& lesson : lessons->sessions) keep_event(lesson.last_try);
        // First noticed recipes/routes and public discoveries/transmissions stay;
        // routine uses/failures retain full facts only while recent or referenced.
        std::set<std::pair<std::uint32_t, std::uint8_t>> firsts;
        std::set<std::uint64_t> permanent;
        std::map<std::uint32_t, std::uint64_t> last_named, last_public;
        for (const auto& event : history.events) {
            if (event.noticed && firsts.insert({event.recipe, event.route}).second) permanent.insert(event.id);
            if (!event.word.empty()) last_named[event.recipe] = event.id;
            if (event.kind >= 1 && event.kind <= 4) last_public[event.recipe] = event.id;
        }
        // Future forgetting/returning cites these exact latest records.
        for (const auto& [recipe, event] : last_named) {
            (void)recipe;
            events.insert(event);
        }
        for (const auto& [recipe, event] : last_public) {
            (void)recipe;
            events.insert(event);
        }
        const auto previous_size = history.events.size();
        history.events.retain([&](const Result& event) {
            const bool public_event = (event.kind >= 1 && event.kind <= 4) || !event.heat_sources.empty();
            return permanent.contains(event.id) || events.contains(event.id) ||
                   event.at >= now - (public_event ? 25 * time::kYear : 2 * time::kDay);
        });
        if (history.events.size() != previous_size) {
            history.public_results.clear();
            history.public_indexed = 0;
            retained_links_.erase(camp);
        }
        history.routine.days.retain([&](const RoutineDay& row) { return row.day >= now - 25 * time::kYear; });
        auto& links = retained_links_[camp];
        if (scalar_work_ || links.indexed > history.events.size()) links = {};
        for (std::size_t n = links.indexed; n < history.events.size(); ++n)
            if (history.events[n].choice) links.event_choices.insert(history.events[n].choice);
        links.indexed = history.events.size();
        if (const auto at = archived_choice_pins_.find(camp); at != archived_choice_pins_.end())
            choices.insert(at->second.begin(), at->second.end());
        history.choices.retain(
            [&](const Choice& choice) {
                return choice.at >= now - 2 * time::kDay || choices.contains(choice.id) ||
                       links.event_choices.contains(choice.id);
            },
            [&](const std::vector<Choice>& page) {
                if (scalar_work_) return false;
                const auto key = std::pair{page.front().id, page.back().id};
                if (links.choice_pages.contains(key)) return true;
                if (!std::all_of(page.begin(), page.end(),
                                 [&](const Choice& choice) { return links.event_choices.contains(choice.id); }))
                    return false;
                links.choice_pages.insert(key);
                return true;
            });
    }
    if (!archive_enabled_) return;
    std::vector<ecs::Id> spent;
    things_.each([&](ecs::Id id, Things::Handle h) {
        const auto* item = things_.raw().try_get<Item>(h);
        const auto* timer = things_.raw().try_get<HeatTimer>(h);
        if (!item || item->mass != 0 || item->state != 4 || physical.contains(id) || things_.raw().all_of<Fire>(h) ||
            (timer && timer->next != 0))
            return;
        archived_ids_.emplace(id, item_archive_.size());
        if (timer && timer->placement_choice) archived_choice_pins_[item->home].insert(timer->placement_choice);
        archive_index_.add(id.value & ((std::uint64_t{1} << 60U) - 1), item_archive_.size());
        item_archive_.push_back(
            {id, now, things_.raw().get<Place>(h), *item,
             timer ? std::make_shared<const HeatTimer>(*timer) : std::shared_ptr<const HeatTimer>{}});
        spent.push_back(id);
    });
    for (const auto id : spent) things_.end(id);
    if (!spent.empty()) {
        item_sites_valid_ = false;
        physical_items_valid_ = false;
    }
}

void World::run_to(time::Seconds goal) {
    item_sites_valid_ = false;
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
    retain_records(goal);
}

void World::run_islands(time::Seconds goal, run::Workers& workers, time::Seconds window) {
    KD_CHECK(goal >= frontier_ && window >= 1, "world::World: islands need a goal ahead and a window");
    for (const auto* system : by_family_) {
        if (system && system->serial_windows(*this)) {
            run_to(goal);
            return;
        }
    }
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
    std::vector<Way> ways;
    std::size_t replaced = 0;
    for (Island& is : islands) {
        history.insert(history.end(), is.history.begin(), is.history.end());
        ways.insert(ways.end(), is.ways.begin(), is.ways.end());
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
    if (ways_list_ != nullptr) {
        num::sort_strict(ways.begin(), ways.end(),
                         [](const Way& x, const Way& y) { return x.key != y.key ? x.key < y.key : x.n < y.n; });
        ways_list_->insert(ways_list_->end(), ways.begin(), ways.end());
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
        beings_.raw().sort<Camp>(
            [&](Beings::Handle a, Beings::Handle b) { return beings_.id_of(a) < beings_.id_of(b); });
        beings_.raw().sort<Person>(
            [&](Beings::Handle a, Beings::Handle b) { return beings_.id_of(a) < beings_.id_of(b); });
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

std::optional<Switch> switch_named(std::string_view name) {
    for (std::size_t i = 0; i < kSwitchNames.size(); ++i) {
        if (kSwitchNames[i] == name) {
            return static_cast<Switch>(i);
        }
    }
    return std::nullopt;
}

void World::set_switches(std::vector<Switch> switches) {
    KD_CHECK(kSwitches || switches.empty(), "world::World: the game's own build has no test switches");
    KD_CHECK(frontier_ == 0 && events_ == 0, "world::World: switches are set before the world runs");
    switches_ = std::move(switches);
}

bool World::keeps(const Record& r) const {
    const ecs::Id owner{r.key.owner};
    const System* s = owner.family() == ecs::Family::none ? by_owner_[owner.value]
                                                          : by_family_[static_cast<std::size_t>(owner.family())];
    return s != nullptr && s->keeps(r.what);
}

Command World::command(time::Seconds at, std::uint32_t what, std::uint64_t a, std::uint64_t b) {
    const Command c{commands_.made_ + 1, at, what, a, b};
    replay(c);
    return c;
}

void World::replay(const Command& c) {
    KD_CHECK(!context_.in_event_ && !in_islands_, "world::World: a command comes from outside the world's events");
    KD_CHECK(c.at >= frontier_, "world::World: a command acts at or after the frontier");
    KD_CHECK(c.number == commands_.made_ + 1, "world::World: commands come in the order of their numbers");
    ++commands_.made_;
    const auto at = std::upper_bound(commands_.pending_.begin(), commands_.pending_.end(), c,
                                     [](const Command& x, const Command& y) { return x.at < y.at; });
    commands_.pending_.insert(at, c);
    commands_.wake();
}

namespace {

// The systems of a world, each once, in the order of their names.
std::vector<System*> systems_of(const std::array<System*, 16>& by_family, const auto& by_owner) {
    std::vector<System*> out;
    for (System* s : by_family) {
        if (s != nullptr && std::find(out.begin(), out.end(), s) == out.end()) {
            out.push_back(s);
        }
    }
    for (System* s : by_owner) {
        if (s != nullptr && std::find(out.begin(), out.end(), s) == out.end()) {
            out.push_back(s);
        }
    }
    num::sort_strict(out.begin(), out.end(), [](const System* x, const System* y) { return x->name() < y->name(); });
    return out;
}

// Each foundation part and its exact current version; older shapes are refused.
constexpr std::array<std::pair<std::uint32_t, std::uint32_t>, 6> kParts{{{save::tag("WRLD"), 2},
                                                                         {save::tag("NAME"), 1},
                                                                         {save::tag("BEIN"), 1},
                                                                         {save::tag("THNG"), 1},
                                                                         {save::tag("QUEU"), 1},
                                                                         {save::tag("SYST"), 1}}};

save::Chunk part(const char (&letters)[5], save::Bytes data) {
    const std::uint32_t t = save::tag(letters);
    const auto it = std::find_if(kParts.begin(), kParts.end(), [&](const auto& p) { return p.first == t; });
    KD_CHECK(it != kParts.end(), "world::World: every part of a snapshot has its version");
    return {t, it->second, true, std::move(data)};
}

}  // namespace

std::vector<save::Chunk> World::save() const {
    KD_CHECK(!context_.in_event_ && !in_islands_, "world::World: a snapshot is taken between events");
    std::vector<save::Chunk> out;
    {
        // the clock: the seed, the frontier, the next id, the world's own owners and the history's count and digest
        ByteWriter w;
        w.u64(seed_);
        w.i64(frontier_);
        w.u64(ids_.next());
        for (std::size_t o = 0; o < owners_.size(); ++o) {
            ecs::write_component(owners_[o], w);
            for (const time::Seconds due : owner_due_[o]) {
                w.i64(due);
            }
        }
        w.u64(history_count_);
        w.u64(history_hash_);
        w.u64(events_);
        w.u64(batches_);
        // version 2: the test switches it runs with, by name
        w.u64(switches_.size());
        for (const Switch s : switches_) {
            w.text(kSwitchNames[static_cast<std::size_t>(s)]);
        }
        out.push_back(part("WRLD", w.take()));
    }
    {
        // the catalogue's names, kind by kind in number order, so the components keep entries by name (A3.6)
        ByteWriter w;
        w.u64(catalogue_.kinds().size());
        for (const auto& k : catalogue_.kinds()) {
            w.text(k->folder());
            w.u64(k->size());
            for (std::size_t i = 0; i < k->size(); ++i) {
                w.text(k->name(i));
            }
        }
        out.push_back(part("NAME", w.take()));
    }
    {
        ByteWriter w;
        beings_.write(w);
        out.push_back(part("BEIN", w.take()));
    }
    {
        ByteWriter w;
        things_.write(w);
        out.push_back(part("THNG", w.take()));
    }
    if (craft_features(*this) != 0) {
        ByteWriter manifest;
        manifest.u64(item_archive_.size());
        manifest.u64(item_archive_.pages().size());
        out.push_back({save::tag("ARCV"), 1, true, manifest.take()});
        const auto encode = [&](const auto& page) {
            ByteWriter wire;
            ecs::PartWriter writer(wire);
            writer.records({"items", "immutable spent identities"}, page, Pages<ArchivedItem>::kPage, 120);
            return wire.take();
        };
        for (std::size_t n = 0; n < item_archive_.pages().size(); ++n)
            out.push_back({save::tag("ARPG"), 1, true, {}, item_archive_.encoded(n, encode)});
        if (!item_archive_.tail().empty()) out.push_back({save::tag("ARPG"), 1, true, encode(item_archive_.tail())});
    }
    {
        ByteWriter w;
        queue_.write(w, [this](const event::Event& e) { return live(e); });
        out.push_back(part("QUEU", w.take()));
    }
    {
        ByteWriter w;
        const std::vector<System*> systems = systems_of(by_family_, by_owner_);
        w.u64(systems.size());
        for (const System* sys : systems) {
            w.text(sys->name());
            ByteWriter own;
            sys->save(own);
            w.blob(own.bytes());
        }
        out.push_back(part("SYST", w.take()));
    }
    // Optional extension components preserve the foundation registry bytes and old proof digests.
    ByteWriter camp;
    std::uint64_t count = 0;
    beings_.each([&](ecs::Id, Beings::Handle h) {
        if (beings_.raw().any_of<Camp, Person>(h)) ++count;
    });
    if (count != 0) {
        camp.u64(count);
        beings_.each([&](ecs::Id id, Beings::Handle h) {
            const auto* place = beings_.raw().try_get<Camp>(h);
            const auto* person = beings_.raw().try_get<Person>(h);
            if (place == nullptr && person == nullptr) return;
            camp.u64(id.value);
            camp.u8(place != nullptr ? 1 : 2);
            if (place != nullptr)
                ecs::write_component(*place, camp);
            else
                ecs::write_component(*person, camp);
        });
        camp.u32(craft_features(*this));
        out.push_back({save::tag("CAMP"), 4U, true, camp.take()});
    }
    if (count != 0) {
        ByteWriter life;
        life.u64(count);
        beings_.each([&](ecs::Id id, Beings::Handle h) {
            if (const auto* env = beings_.raw().try_get<Habitat>(h)) {
                life.u64(id.value);
                life.u8(1);
                ecs::write_component(*env, life);
            } else if (const auto* body = beings_.raw().try_get<Life>(h)) {
                life.u64(id.value);
                life.u8(2);
                ecs::write_component(*body, life);
            }
        });
        out.push_back({save::tag("LIFE"), 2, true, life.take()});
    }
    if (!beings_.raw().view<Dreams>().empty()) {
        ByteWriter dreams;
        dreams.u64(count);
        beings_.each([&](ecs::Id id, Beings::Handle h) {
            if (const auto* ledger = beings_.raw().try_get<Dreams>(h)) {
                dreams.u64(id.value);
                dreams.u8(1);
                dreams.i64(ledger->night);
                for (const auto sent : ledger->sent) dreams.u64(sent);
                dreams.u64(ledger->acts.size());
                for (const auto& act : ledger->acts) ecs::write_component(act, dreams);
                dreams.u64(ledger->ended.size());
                for (const auto& ended : ledger->ended) {
                    dreams.u64(ended.id.value);
                    ecs::write_component(ended.person, dreams);
                    dreams.i64(ended.ended_at);
                    dreams.u64(ended.next_memory);
                }
            } else if (const auto* thought = beings_.raw().try_get<Dream>(h)) {
                dreams.u64(id.value);
                dreams.u8(2);
                ecs::write_component(*thought, dreams);
            }
        });
        out.push_back({save::tag("DRMS"), 3, true, dreams.take()});
    }
    save_craft(*this, out);
    save_fire(*this, out);
    return out;
}

bool World::load(std::span<const save::Chunk> chunks, std::string& why) {
    KD_CHECK(beings_.size() == 0 && things_.size() == 0 && frontier_ == 0 && events_ == 0,
             "world::World: a snapshot is loaded into a world with nothing in it");
    if (std::count_if(chunks.begin(), chunks.end(), [](const auto& c) { return c.tag == save::tag("CAMP"); }) > 1 ||
        std::count_if(chunks.begin(), chunks.end(), [](const auto& c) { return c.tag == save::tag("LIFE"); }) > 1 ||
        std::count_if(chunks.begin(), chunks.end(), [](const auto& c) { return c.tag == save::tag("DRMS"); }) > 1) {
        why = "duplicate Camp alpha records";
        return false;
    }
    const auto* camp_chunk = save::find_chunk(chunks, save::tag("CAMP"));
    const auto* life_chunk = save::find_chunk(chunks, save::tag("LIFE"));
    std::uint32_t features = 0;
    if (!craft_headers(chunks, features, why) || !fire_headers(chunks, features, why)) return false;
    if ((camp_chunk && camp_chunk->version >= 2 && !life_chunk) ||
        (life_chunk && (!camp_chunk || camp_chunk->version < 2))) {
        why = "living camp extension is missing or mismatched";
        return false;
    }
    const auto* dream_chunk = save::find_chunk(chunks, save::tag("DRMS"));
    if ((camp_chunk && camp_chunk->version == 4 && !dream_chunk) ||
        (dream_chunk &&
         (!camp_chunk || camp_chunk->version != 4 || !dream_chunk->critical || dream_chunk->version != 3))) {
        why = "dream extension is missing or mismatched";
        return false;
    }
    // each part brought up to the version this one writes; a part it does not know is skipped, unless it must be known
    for (const save::Chunk& c : chunks) {
        if (c.critical && c.tag != save::tag("CAMP") && c.tag != save::tag("LIFE") && c.tag != save::tag("DRMS") &&
            c.tag != save::tag("CRFT") && c.tag != save::tag("KNOW") && c.tag != save::tag("HIST") &&
            c.tag != save::tag("EVPG") && c.tag != save::tag("CHPG") && c.tag != save::tag("ARPG") &&
            c.tag != save::tag("RTPG") && c.tag != save::tag("ARCV") && c.tag != save::tag("LEAR") &&
            c.tag != save::tag("FIRE") && c.tag != save::tag("THER") &&
            std::none_of(kParts.begin(), kParts.end(), [&](const auto& p) { return p.first == c.tag; })) {
            why = "it holds a part this version cannot read";
            return false;
        }
    }
    std::vector<save::Chunk> parts;
    for (const auto& [t, version] : kParts) {
        const save::Chunk* c = save::find_chunk(chunks, t);
        if (c == nullptr) {
            why = "a part of it is missing";
            return false;
        }
        parts.push_back(*c);
        if (parts.back().version != version) {
            why = "unsupported foundation chunk version";
            return false;
        }
    }
    const auto chunk_of = [&](std::uint32_t t) -> const save::Chunk& { return *save::find_chunk(parts, t); };

    // the snapshot's names: from its numbers for each kind's entries to this catalogue's
    std::vector<std::pair<std::string, std::vector<std::string>>> names;
    {
        ByteReader r(chunk_of(save::tag("NAME")).data);
        std::uint64_t kinds = 0;
        if (!r.u64(kinds)) {
            why = "its names are damaged";
            return false;
        }
        for (std::uint64_t k = 0; k < kinds; ++k) {
            std::string folder;
            std::uint64_t n = 0;
            if (!r.text(folder) || !r.u64(n)) {
                why = "its names are damaged";
                return false;
            }
            std::vector<std::string> list;
            for (std::uint64_t i = 0; i < n && !r.failed(); ++i) {
                list.emplace_back();
                r.text(list.back());
            }
            names.emplace_back(std::move(folder), std::move(list));
        }
        if (!r.finished()) {
            why = "its names are damaged";
            return false;
        }
    }
    const ecs::EntryMap entries = [&](std::string_view folder, std::uint32_t saved) -> std::optional<std::uint32_t> {
        for (const auto& [f, list] : names) {
            if (f == folder) {
                return saved < list.size() ? catalogue_.find(f, list[saved]) : std::nullopt;
            }
        }
        return std::nullopt;
    };

    {
        ByteReader r(chunk_of(save::tag("WRLD")).data);
        std::uint64_t next = 0;
        bool ok = r.u64(seed_) && r.i64(frontier_) && r.u64(next);
        for (std::size_t o = 0; ok && o < owners_.size(); ++o) {
            ok = ecs::read_component(owners_[o], r, entries);
            for (time::Seconds& due : owner_due_[o]) {
                ok = ok && r.i64(due);
            }
        }
        ok = ok && r.u64(history_count_) && r.u64(history_hash_) && r.u64(events_) && r.u64(batches_);
        std::uint64_t switches = 0;
        ok = ok && r.u64(switches);
        for (std::uint64_t i = 0; ok && i < switches; ++i) {
            std::string name;
            const std::optional<Switch> s = r.text(name) ? switch_named(name) : std::nullopt;
            ok = s.has_value();
            if (ok) {
                switches_.push_back(*s);
            }
        }
        if (!ok || !r.finished() || frontier_ < 0 || next == 0) {
            why = "its clock is damaged, or names a test switch this version does not know";
            return false;
        }
        ids_ = ecs::IdMaker(next);
    }
    {
        ByteReader r(chunk_of(save::tag("BEIN")).data);
        if (!beings_.read(r, entries) || !r.finished()) {
            why = "its beings are damaged, or of kinds the catalogue no longer has";
            return false;
        }
    }
    {
        ByteReader r(chunk_of(save::tag("THNG")).data);
        if (!things_.read(r, entries) || !r.finished()) {
            why = "its things are damaged, or of kinds the catalogue no longer has";
            return false;
        }
    }
    {
        if (const save::Chunk* camp = save::find_chunk(chunks, save::tag("CAMP"))) {
            ByteReader records(camp->data);
            std::uint64_t count = 0;
            ecs::Id last{};
            if (camp->version != 4 || !records.u64(count) || count > beings_.size()) {
                why = "invalid Camp alpha records";
                return false;
            }
            for (std::uint64_t i = 0; i < count; ++i) {
                ecs::Id id{};
                std::uint8_t kind = 0;
                if (!records.u64(id.value) || !(last < id) || !records.u8(kind)) {
                    why = "invalid Camp alpha identity order";
                    return false;
                }
                last = id;
                const auto h = beings_.find(id);
                if (!h || !beings_.raw().all_of<Place, Schedule>(*h)) {
                    why = "Camp alpha record has no entity";
                    return false;
                }
                const auto at = beings_.raw().get<Place>(*h).at;
                if (at.x < 0 || at.x >= torus_.width() || at.y < 0 || at.y >= torus_.height()) {
                    why = "Camp alpha position outside world coordinate range";
                    return false;
                }
                if (kind == 1 && id.family() == ecs::Family::place) {
                    Camp value;
                    if (!ecs::read_component(value, records, entries) || value.half_width_cm < 1 ||
                        value.half_width_cm > 100000 || value.half_height_cm < 1 || value.half_height_cm > 100000 ||
                        value.water_ml < 0 || value.food_mg < 0 || value.stone_mg < 0 || value.wood_mg < 0) {
                        why = "invalid Camp alpha supplies or bounds";
                        return false;
                    }
                    const auto centre = beings_.raw().get<Place>(*h).at;
                    for (const auto site :
                         {value.water_at, value.food_at, value.stone_at, value.wood_at, value.shelter_at}) {
                        const auto offset = torus_.offset(centre, site);
                        if (site.x < 0 || site.x >= torus_.width() || site.y < 0 || site.y >= torus_.height() ||
                            offset.dx < -value.half_width_cm || offset.dx > value.half_width_cm ||
                            offset.dy < -value.half_height_cm || offset.dy > value.half_height_cm) {
                            why = "Camp alpha supply outside its patch";
                            return false;
                        }
                    }
                    beings_.raw().emplace<Camp>(*h, value);
                } else if (kind == 2 && id.family() == ecs::Family::person) {
                    Person value;
                    if (!ecs::read_component(value, records, entries) || value.name_index >= kPersonNames.size() ||
                        value.age_years < 18 || value.age_years > 45 || value.appearance > 24 ||
                        !beings_.raw().all_of<Activity, demo::Home>(*h)) {
                        why = "invalid Camp alpha person";
                        return false;
                    }
                    const auto& idle = beings_.raw().get<Activity>(*h);
                    const auto& schedule = beings_.raw().get<Schedule>(*h);
                    if (!save::find_chunk(chunks, save::tag("LIFE")) &&
                        (idle.what != 0 || idle.from != idle.to || idle.from != beings_.raw().get<Place>(*h).at ||
                         std::any_of(schedule.expected.begin(), schedule.expected.end(),
                                     [](auto sequence) { return sequence != 0; }))) {
                        why = "Camp alpha person is not an idle record";
                        return false;
                    }
                    beings_.raw().emplace<Person>(*h, value);
                } else {
                    why = "invalid Camp alpha record kind";
                    return false;
                }
            }
            std::uint32_t saved_features = 0;
            if (!records.u32(saved_features) || saved_features != features || !records.finished()) {
                why = "trailing Camp alpha records";
                return false;
            }
        }
        if (const save::Chunk* life = save::find_chunk(chunks, save::tag("LIFE"))) {
            ByteReader r(life->data);
            std::uint64_t count = 0;
            ecs::Id last{};
            if (life->version != 2 || !r.u64(count) || count > beings_.size()) {
                why = "invalid living camp records";
                return false;
            }
            for (std::uint64_t i = 0; i < count; ++i) {
                ecs::Id id{};
                std::uint8_t kind = 0;
                if (!r.u64(id.value) || !(last < id) || !r.u8(kind)) {
                    why = "invalid living identity order";
                    return false;
                }
                last = id;
                const auto h = beings_.find(id);
                if (!h) {
                    why = "living record has no entity";
                    return false;
                }
                if (kind == 1 && beings_.raw().all_of<Camp>(*h)) {
                    Habitat env;
                    if (!ecs::read_component(env, r, entries)) {
                        why = "damaged habitat";
                        return false;
                    }
                    const auto& patch = beings_.raw().get<Camp>(*h);
                    const std::array<std::int64_t, 12> amounts{env.upstream_ml,  env.root_water_ml, env.crop_budget_mg,
                                                               env.water_cap_ml, env.food_cap_mg,   env.renewed_at,
                                                               env.water_added,  env.food_grown,    env.food_taken,
                                                               env.water_taken,  env.water_spilled, frontier_};
                    if (patch.half_width_cm > 5000 || patch.half_height_cm > 5000 || env.rock_west > env.rock_east ||
                        env.rock_south > env.rock_north || env.rock_west < -patch.half_width_cm ||
                        env.rock_east > patch.half_width_cm || env.rock_south < -patch.half_height_cm ||
                        env.rock_north > patch.half_height_cm || env.renewed_at > frontier_ ||
                        patch.food_mg > env.food_cap_mg || patch.water_ml > env.water_cap_ml ||
                        std::any_of(amounts.begin(), amounts.end(),
                                    [](auto v) { return v < 0 || v > 1000000000000LL; })) {
                        why = "invalid habitat inputs or bounds";
                        return false;
                    }
                    beings_.raw().emplace<Habitat>(*h, env);
                } else if (kind == 2 && beings_.raw().all_of<Person>(*h)) {
                    Life body;
                    if (!ecs::read_component(body, r, entries)) {
                        why = "damaged person life";
                        return false;
                    }
                    const auto& act = beings_.raw().get<Activity>(*h);
                    const auto& schedule = beings_.raw().get<Schedule>(*h);
                    const auto is_point = [&](num::Point p) {
                        return p.x >= 0 && p.x < torus_.width() && p.y >= 0 && p.y < torus_.height();
                    };
                    if (body.food < 0 || body.food > 4000000 || body.water < 0 || body.water > 3000 || body.awake < 0 ||
                        body.awake > 129600 || body.settled < 0 || body.settled > frontier_ ||
                        body.food_remainder < 0 || body.food_remainder >= time::kDay || body.water_remainder < 0 ||
                        body.water_remainder >= time::kDay || body.food_water_remainder < 0 ||
                        body.food_water_remainder >= 1000000 || body.food_factor_ppm < 0 ||
                        body.food_factor_ppm > 2500000 || body.water_ml_per_kg < 0 || body.water_ml_per_kg > 1000 ||
                        body.nutrient_remainder < 0 || body.nutrient_remainder >= 1000000 ||
                        (features == 0 && (body.meal_item.value != 0 || act.what > 7)) || body.goal > 3 ||
                        body.gathering_skill > 10 || body.carried_food < 0 || body.carried_food > 4000000 ||
                        body.allocated_water < 0 || body.allocated_water > 3000 || body.portion < 0 ||
                        body.portion > 4000000 || body.applied < 0 || body.applied > body.portion ||
                        body.decision_at < 0 || body.decision_at > frontier_ || body.notice_at < -3600 ||
                        body.notice_at > frontier_ || body.memory_at < -1 || body.memory_at > frontier_ ||
                        body.memory_kind < 0 || body.memory_kind > 12 || body.memory_amount < 0 ||
                        body.memory_amount > 4000000 || act.what > 12 || act.what == 3 || act.start != body.settled ||
                        act.start > frontier_ || act.end < frontier_ || act.end <= act.start ||
                        act.end - act.start > time::kDay || !is_point(act.from) || !is_point(act.to) ||
                        !is_point(body.explore_at) || !is_point(body.use_at) || schedule.expected[kActivitySlot] == 0 ||
                        (features == 0 && schedule.expected[2] != 0) || schedule.expected[3] != 0 ||
                        (act.what == 5 && body.portion - body.applied > body.carried_food) ||
                        (act.what == 6 && body.allocated_water != body.portion - body.applied) ||
                        (act.what != 6 && body.allocated_water != 0) || act.from != beings_.raw().get<Place>(*h).at) {
                        why = "invalid needs or pending living action";
                        return false;
                    }
                    for (std::size_t n = 0; n < 3; ++n) {
                        if (body.source[n] > 3 || body.unavailable[n] > 5 || body.seen[n] < -1 ||
                            body.seen[n] > frontier_ || body.known_amount[n] < 0 ||
                            body.known_amount[n] > 1000000000000LL || body.blocked_until[n] < 0 ||
                            body.blocked_until[n] > frontier_ + 3600 || body.decision_needs[n] < 0 ||
                            body.decision_needs[n] > 100 || body.benefit[n] < 0 || body.benefit[n] > 100 ||
                            body.cost_seconds[n] < 0 || body.cost_seconds[n] > time::kDay * 2 ||
                            (body.source[n] != 0 && !is_point(body.known_at[n]))) {
                            why = "invalid remembered supplies";
                            return false;
                        }
                    }
                    if (std::any_of(body.scores.begin(), body.scores.end(),
                                    [](auto v) { return v < -1000000 || v > 100000; })) {
                        why = "invalid recorded decision";
                        return false;
                    }
                    beings_.raw().emplace<Life>(*h, body);
                } else {
                    why = "invalid living record kind";
                    return false;
                }
            }
            if (!r.finished()) {
                why = "trailing living records";
                return false;
            }
        }
        bool valid_people = true;
        beings_.each([&](ecs::Id id, Beings::Handle h) {
            if (id.family() != ecs::Family::person) return;
            const auto* person = beings_.raw().try_get<Person>(h);
            const auto* home = beings_.raw().try_get<demo::Home>(h);
            const auto camp = home != nullptr ? beings_.find(home->camp) : std::nullopt;
            if (person == nullptr || !camp || !beings_.raw().all_of<Camp, Place>(*camp) ||
                (save::find_chunk(chunks, save::tag("LIFE")) &&
                 (!beings_.raw().all_of<Life>(h) || !beings_.raw().all_of<Habitat>(*camp)))) {
                valid_people = false;
                return;
            }
            if (home->at != beings_.raw().get<Place>(*camp).at) {
                valid_people = false;
                return;
            }
            const auto centre = beings_.raw().get<Place>(*camp).at;
            const auto person_at = beings_.raw().get<Place>(h).at;
            const auto offset = torus_.offset(centre, person_at);
            const auto& bounds = beings_.raw().get<Camp>(*camp);
            if (offset.dx < -bounds.half_width_cm || offset.dx > bounds.half_width_cm ||
                offset.dy < -bounds.half_height_cm || offset.dy > bounds.half_height_cm)
                valid_people = false;
        });
        if (!valid_people) {
            why = "Camp alpha person missing or outside its patch";
            return false;
        }
        if (save::find_chunk(chunks, save::tag("LIFE"))) {
            bool valid_paths = true;
            beings_.each([&](ecs::Id id, Beings::Handle h) {
                if (id.family() != ecs::Family::person) return;
                const auto& life = beings_.raw().get<Life>(h);
                const auto& home = beings_.raw().get<demo::Home>(h);
                const auto ch = beings_.handle(home.camp);
                const auto centre = beings_.raw().get<Place>(ch).at;
                const auto& patch = beings_.raw().get<Camp>(ch);
                const auto& rock = beings_.raw().get<Habitat>(ch);
                const auto inside = [&](num::Point point) {
                    const auto offset = torus_.offset(centre, point);
                    return std::abs(offset.dx) <= patch.half_width_cm && std::abs(offset.dy) <= patch.half_height_cm;
                };
                const auto& act = beings_.raw().get<Activity>(h);
                if (!inside(act.from) || !inside(act.to) || !inside(life.explore_at) || !inside(life.use_at) ||
                    (act.what != 1 && act.what != 7 && act.from != act.to))
                    valid_paths = false;
                const auto offset = torus_.offset(centre, act.from);
                if (!camp_line_clear(torus_.offset(centre, act.from), torus_.offset(centre, act.to), rock))
                    valid_paths = false;
                if (offset.dx >= rock.rock_west && offset.dx <= rock.rock_east && offset.dy >= rock.rock_south &&
                    offset.dy <= rock.rock_north)
                    valid_paths = false;
                if (valid_paths &&
                    !camp_walk_line_clear(torus_.offset(centre, act.from), torus_.offset(centre, act.to), rock)) {
                    // The sufficient geometric test can be conservative near
                    // an edge. Validate the actual saved integer-time motion,
                    // rather than rejecting a legal trajectory. LIFE already
                    // bounds this interval to one day before this loop.
                    for (auto at = act.start; at < act.end; ++at) {
                        const auto point = torus_.offset(centre, act.at(torus_, at));
                        if (!camp_line_clear(point, point, rock)) {
                            valid_paths = false;
                            break;
                        }
                    }
                }
                for (std::size_t n = 0; n < 3; ++n)
                    if (life.source[n] != 0 && !inside(life.known_at[n])) valid_paths = false;
            });
            if (!valid_paths) {
                why = "living path or memory outside reachable camp";
                return false;
            }
        }
        ByteReader r(chunk_of(save::tag("QUEU")).data);
        std::optional<event::Queue> q = event::Queue::read(r);
        if (!q || !r.finished()) {
            why = "its events are damaged";
            return false;
        }
        for (const auto& event : q->live_in_order([](const auto&) { return true; })) {
            if (event.slot >= Schedule::kSlots || event.key.second < frontier_) {
                why = "invalid event slot or past event";
                return false;
            }
        }
        queue_ = std::move(*q);
        if (save::find_chunk(chunks, save::tag("LIFE"))) {
            const auto events = queue_.live_in_order([](const auto&) { return true; });
            bool valid = true;
            for (const auto& event : events) {
                const ecs::Id id{event.key.owner};
                if (id.family() != ecs::Family::person && id.family() != ecs::Family::place) continue;
                const auto h = beings_.find(id);
                if (!h || event.slot >= Schedule::kSlots || !beings_.raw().any_of<Life, Habitat>(*h)) valid = false;
            }
            beings_.each([&](ecs::Id id, Beings::Handle h) {
                if (!beings_.raw().any_of<Camp, Person>(h)) return;
                if ((id.family() == ecs::Family::place && !beings_.raw().all_of<Habitat>(h))) {
                    valid = false;
                    return;
                }
                const auto& schedule = beings_.raw().get<Schedule>(h);
                for (std::uint32_t slot = 0; slot < Schedule::kSlots; ++slot) {
                    std::size_t matches = 0;
                    for (const auto& event : events) {
                        if (event.key.owner != id.value || event.slot != slot) continue;
                        if (event.key.sequence != schedule.expected[slot] || event.key.second < frontier_) {
                            valid = false;
                            continue;
                        }
                        ++matches;
                        if (slot == kActivitySlot) {
                            const auto due = id.family() == ecs::Family::person
                                                 ? beings_.raw().get<Activity>(h).end
                                                 : beings_.raw().get<Habitat>(h).renewed_at + 3600;
                            if (event.key.second != due) valid = false;
                        }
                    }
                    if (matches != (schedule.expected[slot] != 0 ? 1U : 0U)) valid = false;
                }
            });
            if (!valid) {
                why = "living action has no matching live event";
                return false;
            }
        }
    }
    if (dream_chunk) {
        ByteReader r(dream_chunk->data);
        std::uint64_t count = 0;
        std::uint64_t expected = 0;
        beings_.each([&](ecs::Id, Beings::Handle h) {
            if (beings_.raw().any_of<Camp, Person>(h)) ++expected;
        });
        if (!r.u64(count) || count != expected) {
            why = "missing dream records";
            return false;
        }
        ecs::Id last{};
        for (std::uint64_t i = 0; i < count; ++i) {
            ecs::Id id{};
            std::uint8_t kind = 0;
            if (!r.u64(id.value) || !(last < id) || !r.u8(kind)) {
                why = "invalid dream identity order";
                return false;
            }
            last = id;
            const auto h = beings_.find(id);
            if (!h) {
                why = "dream record has no entity";
                return false;
            }
            const auto home = kind == 1                              ? id
                              : beings_.raw().all_of<demo::Home>(*h) ? beings_.raw().get<demo::Home>(*h).camp
                                                                     : ecs::Id{};
            const auto ch = beings_.find(home);
            if (!ch || !beings_.raw().all_of<Camp, Place>(*ch)) {
                why = "dream record has no camp";
                return false;
            }
            const auto& patch = beings_.raw().get<Camp>(*ch);
            const auto centre = beings_.raw().get<Place>(*ch).at;
            const auto inside = [&](num::Point p) {
                if (p.x < 0 || p.x >= torus_.width() || p.y < 0 || p.y >= torus_.height()) return false;
                const auto offset = torus_.offset(centre, p);
                return std::abs(offset.dx) <= patch.half_width_cm && std::abs(offset.dy) <= patch.half_height_cm;
            };
            const auto idea_valid = [&](const IdeaFields& idea, time::Seconds delivered, bool pending) {
                if (idea.kind > 1 || idea.action > 20 || idea.desired_property >= 18 || idea.first_attempt_at < -1 ||
                    idea.first_attempt_at > frontier_)
                    return false;
                if (idea.kind == 0)
                    return idea.memory == 0 && idea.hunch_id == 0 && idea.action == 0 && idea.desired_property == 0 &&
                           idea.recipe == kNoRecipe && idea.inputs.empty() && idea.first_attempt_at == -1;
                if (!(features & kIdeas) || idea.memory == 0 || idea.inputs.empty() || idea.inputs.size() > 2 ||
                    (pending && (idea.hunch_id != 0 || idea.first_attempt_at != -1 || idea.recipe == kNoRecipe)) ||
                    (!pending && delivered >= 0 && idea.hunch_id != idea.memory) ||
                    (idea.first_attempt_at >= 0 && (delivered < 0 || idea.first_attempt_at <= delivered)))
                    return false;
                const auto& recipes = catalogue_.kind<data::Blueprint>();
                if (idea.recipe != kNoRecipe &&
                    (idea.recipe >= recipes.size() || recipes[idea.recipe].action != idea.action))
                    return false;
                return true;
            };
            const auto night = frontier_ / time::kDay + (frontier_ % time::kDay + 18 * time::kHour) / time::kDay - 1;
            if (kind == 1 && beings_.raw().all_of<Camp>(*h)) {
                Dreams ledger;
                std::uint64_t n = 0;
                if (!r.i64(ledger.night) || ledger.night < -2 || ledger.night > night) {
                    why = "invalid dream night";
                    return false;
                }
                for (auto& sent : ledger.sent)
                    if (!r.u64(sent)) {
                        why = "damaged nightly dream cap";
                        return false;
                    }
                std::vector<std::uint64_t> used;
                bool empty = false;
                for (const auto sent : ledger.sent) {
                    if (sent == 0) {
                        empty = true;
                        continue;
                    }
                    if (empty || ecs::Id{sent}.family() != ecs::Family::person ||
                        std::find(used.begin(), used.end(), sent) != used.end()) {
                        why = "invalid nightly dream cap";
                        return false;
                    }
                    used.push_back(sent);
                }
                if (!r.u64(n) || n > 1000000 || n > dream_chunk->data.size() / 100) {
                    why = "oversized dream ledger";
                    return false;
                }
                std::uint64_t previous = 0;
                std::vector<std::uint64_t> pending;
                std::map<std::int64_t, std::vector<std::uint64_t>> delivered;
                for (std::uint64_t a = 0; a < n; ++a) {
                    DreamAct act;
                    if (!ecs::read_component(act, r, entries) || act.number <= previous ||
                        ecs::Id{act.person}.family() != ecs::Family::person ||
                        (act.person & ((std::uint64_t{1} << 60U) - 1)) == 0 ||
                        (act.person & ((std::uint64_t{1} << 60U) - 1)) >= ids_.next() ||
                        (act.kind == 0 ? (act.subject < 0 || act.subject > 2) : act.subject != -1) ||
                        !idea_valid(act, act.status == 2 ? act.executed : -1, act.status == 1) || !inside(act.place) ||
                        act.requested < 0 || act.requested > frontier_ || act.received != act.requested ||
                        act.status < 1 || act.status > 3 || act.reason > 5 || act.executed < -1 ||
                        act.executed > frontier_ ||
                        act.executed > std::numeric_limits<std::int64_t>::max() - 3 * time::kDay ||
                        act.decision_at < -1 || act.decision_at > frontier_ || act.visited_at < -1 ||
                        act.visited_at > frontier_ || act.choice < -1 || act.choice > 4 || act.pull < 0 ||
                        (act.pull != 0 && act.pull != 60) || (act.decision_at == -1 && act.pull != 0) ||
                        ((act.decision_at == -1) != (act.choice == -1)) ||
                        (act.status == 1 && (act.executed != -1 || act.until != -1 || act.reason != 0 ||
                                             act.decision_at != -1 || act.visited_at != -1)) ||
                        (act.status == 2 && (act.executed < act.received ||
                                             act.until != act.executed + 3 * time::kDay || act.reason != 0)) ||
                        (act.status == 3 && (act.executed < act.received || act.until != -1 || act.reason == 0)) ||
                        (act.decision_at != -1 && (act.status != 2 || act.decision_at < act.executed ||
                                                   act.decision_at >= act.until || act.choice == -1)) ||
                        (act.visited_at != -1 &&
                         (act.status != 2 || act.visited_at < act.executed || act.visited_at >= act.until))) {
                        why = "invalid private dream act";
                        return false;
                    }
                    previous = act.number;
                    if (act.status == 1) {
                        if (pending.size() == 3 ||
                            std::find(pending.begin(), pending.end(), act.person) != pending.end()) {
                            why = "uncapped queued dreams";
                            return false;
                        }
                        pending.push_back(act.person);
                    }
                    if (act.status == 2) {
                        const auto acted_night = (act.executed + 18 * time::kHour) / time::kDay - 1;
                        auto& sleepers = delivered[acted_night];
                        if (acted_night > ledger.night || sleepers.size() == 3 ||
                            std::find(sleepers.begin(), sleepers.end(), act.person) != sleepers.end()) {
                            why = "uncapped delivered dreams";
                            return false;
                        }
                        sleepers.push_back(act.person);
                    }
                    ledger.acts.push_back(act);
                }
                if (!r.u64(n) || n > dream_chunk->data.size() / 36) {
                    why = "oversized ended-person ledger";
                    return false;
                }
                ecs::Id last_ended{};
                for (std::uint64_t a = 0; a < n; ++a) {
                    Dreams::EndedPerson ended;
                    if (!r.u64(ended.id.value) || !(last_ended < ended.id) ||
                        ended.id.family() != ecs::Family::person ||
                        (ended.id.value & ((std::uint64_t{1} << 60U) - 1)) == 0 ||
                        (ended.id.value & ((std::uint64_t{1} << 60U) - 1)) >= ids_.next() || beings_.find(ended.id) ||
                        !ecs::read_component(ended.person, r, entries) ||
                        ended.person.name_index >= kPersonNames.size() || ended.person.age_years < 18 ||
                        ended.person.age_years > 45 || ended.person.appearance > 24 || !r.i64(ended.ended_at) ||
                        ended.ended_at < 0 || ended.ended_at > frontier_ || !r.u64(ended.next_memory) ||
                        ((features != 0) != (ended.next_memory != 0))) {
                        why = "invalid typed ended-person record";
                        return false;
                    }
                    last_ended = ended.id;
                    ledger.ended.push_back(ended);
                }
                auto expected_sent = delivered[ledger.night];
                std::stable_sort(expected_sent.begin(), expected_sent.end());
                std::stable_sort(used.begin(), used.end());
                if (used != expected_sent) {
                    why = "nightly dream cap disagrees with delivered dreams";
                    return false;
                }
                beings_.raw().emplace<Dreams>(*h, std::move(ledger));
            } else if (kind == 2 && beings_.raw().all_of<Person>(*h)) {
                Dream thought;
                if (!ecs::read_component(thought, r, entries) || thought.night < -2 || thought.night > night ||
                    thought.at < -1 || thought.at > frontier_ ||
                    thought.at > std::numeric_limits<std::int64_t>::max() - 3 * time::kDay || thought.subject < -1 ||
                    thought.subject > 2 || !idea_valid(thought, thought.at, false) ||
                    (thought.decision_pull != 0 && thought.decision_pull != 60) || thought.decision_subject < -1 ||
                    thought.decision_subject > 2 ||
                    (thought.kind == 0 && ((thought.decision_pull == 0) != (thought.decision_subject == -1))) ||
                    (thought.kind == 1 && thought.decision_subject != -1) || thought.visit_at < -1 ||
                    thought.visit_at > frontier_ ||
                    (thought.at == -1 && (thought.subject != -1 || thought.until != -1 || thought.decision_pull != 0 ||
                                          thought.visit_at != -1)) ||
                    (thought.at >= 0 &&
                     (thought.until != thought.at + 3 * time::kDay || !inside(thought.place) ||
                      (thought.at + 18 * time::kHour) / time::kDay - 1 > thought.night ||
                      (thought.kind == 0 &&
                       (thought.subject == -1 ||
                        beings_.raw().get<Life>(*h).source[static_cast<std::size_t>(thought.subject)] == 0 ||
                        thought.place !=
                            beings_.raw().get<Life>(*h).known_at[static_cast<std::size_t>(thought.subject)])) ||
                      (thought.kind == 1 && thought.subject != -1))) ||
                    (thought.visit_at != -1 && thought.visit_at < thought.at)) {
                    why = "invalid ordinary dream thought";
                    return false;
                }
                beings_.raw().emplace<Dream>(*h, thought);
            } else {
                why = "invalid dream record kind";
                return false;
            }
        }
        if (!r.finished()) {
            why = "trailing dream records";
            return false;
        }
    }
    {
        ByteReader r(chunk_of(save::tag("SYST")).data);
        const std::vector<System*> systems = systems_of(by_family_, by_owner_);
        std::uint64_t n = 0;
        if (!r.u64(n) || n != systems.size()) {
            why = "it was saved with other systems than this version's";
            return false;
        }
        for (System* sys : systems) {
            std::string name;
            std::vector<std::byte> own;
            if (!r.text(name) || name != sys->name() || !r.blob(own)) {
                why = "it was saved with other systems than this version's";
                return false;
            }
            ByteReader sr(own);
            if (!sys->load(sr)) {
                why = "the state of its " + name + " is damaged";
                return false;
            }
        }
        if (!r.finished()) {
            why = "its systems are damaged";
            return false;
        }
        for (const auto h : beings_.raw().view<Dreams>()) {
            for (const auto& act : beings_.raw().get<Dreams>(h).acts) {
                if (act.number > commands_.made_) {
                    why = "dream has no requested command";
                    return false;
                }
            }
        }
        if (features != 0) {
            const auto* manifest = save::find_chunk(chunks, save::tag("ARCV"));
            std::uint64_t total = 0, pages = 0, count = 0;
            if (!manifest || manifest->version != 1 || !manifest->critical) {
                why = "missing or unsupported spent archive";
                return false;
            }
            ByteReader header(manifest->bytes());
            if (!header.u64(total) || !header.u64(pages) || !header.finished() || pages > chunks.size()) {
                why = "damaged spent archive manifest";
                return false;
            }
            for (const auto& chunk : chunks) {
                if (chunk.tag != save::tag("ARPG")) continue;
                ByteReader wire(chunk.bytes());
                ecs::PartReader reader(wire, entries);
                std::vector<ArchivedItem> page;
                reader.records({"items", "immutable spent identities"}, page, Pages<ArchivedItem>::kPage, 120);
                if (chunk.version != 1 || !chunk.critical || !reader.ok() || !wire.finished() || page.empty()) {
                    why = "damaged spent archive page";
                    return false;
                }
                for (const auto& archived : page) {
                    const auto serial = archived.id.value & ((std::uint64_t{1} << 60U) - 1);
                    if (archived.archived_at < 0 || archived.archived_at > frontier_ ||
                        archived.id.family() != ecs::Family::thing || serial == 0 || serial >= ids_.next() ||
                        archived.item.mass != 0 || archived.item.state != 4 || things_.find(archived.id) ||
                        !archived_ids_.emplace(archived.id, item_archive_.size() + (&archived - page.data())).second ||
                        archived.has_timer > 1 || (archived.timer && archived.timer->next != 0)) {
                        why = "invalid spent archive identity or active quantity";
                        return false;
                    }
                    archive_index_.add(serial,
                                       item_archive_.size() + static_cast<std::size_t>(&archived - page.data()));
                    if (archived.timer && archived.timer->placement_choice)
                        archived_choice_pins_[archived.item.home].insert(archived.timer->placement_choice);
                }
                if (count < pages && page.size() != Pages<ArchivedItem>::kPage) {
                    why = "incomplete sealed spent page";
                    return false;
                }
                if (count++ < pages)
                    item_archive_.append_page(std::make_shared<const std::vector<ArchivedItem>>(std::move(page)));
                else
                    for (auto& archived : page) item_archive_.push_back(std::move(archived));
            }
            const auto tail =
                total >= pages * Pages<ArchivedItem>::kPage ? total - pages * Pages<ArchivedItem>::kPage : 0;
            if (item_archive_.size() != total || tail > Pages<ArchivedItem>::kPage ||
                count != pages + (tail ? 1U : 0U)) {
                why = "missing spent archive page";
                return false;
            }
        }
        if (!load_craft(*this, chunks, entries, features, why) || !load_fire(*this, chunks, entries, features, why))
            return false;
        for (const auto ph : beings_.raw().view<Dream, Knowledge>()) {
            const auto& thought = beings_.raw().get<Dream>(ph);
            if (thought.kind == 1 && thought.memory >= beings_.raw().get<Knowledge>(ph).next_memory) {
                why = "idea has an invented memory identity";
                return false;
            }
        }
        std::map<std::uint64_t, ecs::Id> allocated;
        beings_.each([&](ecs::Id id, auto) { allocated.emplace(id.value & ((std::uint64_t{1} << 60U) - 1), id); });
        things_.each([&](ecs::Id id, auto) { allocated.emplace(id.value & ((std::uint64_t{1} << 60U) - 1), id); });
        for (const auto& archived : item_archive_)
            if (!allocated.emplace(archived.id.value & ((std::uint64_t{1} << 60U) - 1), archived.id).second) {
                why = "spent serial already identifies another record";
                return false;
            }
        for (const auto ch : beings_.raw().view<Dreams>()) {
            const auto& ledger = beings_.raw().get<Dreams>(ch);
            for (const auto& ended : ledger.ended) {
                if (!allocated.emplace(ended.id.value & ((std::uint64_t{1} << 60U) - 1), ended.id).second) {
                    why = "ended-person serial already identifies another record";
                    return false;
                }
            }
            for (const auto& act : ledger.acts) {
                const auto target = beings_.find(ecs::Id{act.person});
                const auto ended =
                    std::lower_bound(ledger.ended.begin(), ledger.ended.end(), ecs::Id{act.person},
                                     [](const auto& entry, ecs::Id wanted) { return entry.id < wanted; });
                if (!target &&
                    (ended == ledger.ended.end() || ended->id.value != act.person || act.requested > ended->ended_at ||
                     (act.status == 2 && act.executed > ended->ended_at))) {
                    why = "private dream target has no live or typed ended-person identity";
                    return false;
                }
                if (target && (!beings_.raw().all_of<Person, demo::Home>(*target) ||
                               beings_.raw().get<demo::Home>(*target).camp != beings_.id_of(ch))) {
                    why = "private dream target is not a person of its camp";
                    return false;
                }
                if (act.kind == 1 && target && beings_.raw().all_of<Knowledge>(*target) &&
                    act.memory >= beings_.raw().get<Knowledge>(*target).next_memory) {
                    why = "private idea has an invented memory identity";
                    return false;
                }
                if (act.kind == 1 && !target && act.memory >= ended->next_memory) {
                    why = "ended person's idea has an invented memory identity";
                    return false;
                }
            }
        }
        context_.now_ = frontier_;
        for (System* sys : systems) {
            sys->opened(*this);
        }
    }
    return true;
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
        // a world run with test switches is told apart; one without digests as it always has
        if (!switches_.empty()) {
            d.u64(switches_.size());
            for (const Switch s : switches_) {
                d.text(kSwitchNames[static_cast<std::size_t>(s)]);
            }
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
        beings_.each([&](ecs::Id id, Beings::Handle h) {
            if (const auto* c = beings_.raw().try_get<Camp>(h)) {
                d.u64(id.value);
                ecs::digest_component(*c, d);
            }
            if (const auto* life = beings_.raw().try_get<Life>(h)) ecs::digest_component(*life, d);
            if (const auto* thermal = beings_.raw().try_get<Thermal>(h)) ecs::digest_component(*thermal, d);
            if (const auto* ambient = beings_.raw().try_get<Ambient>(h)) ecs::digest_component(*ambient, d);
            if (const auto* work = beings_.raw().try_get<Work>(h)) ecs::digest_component(*work, d);
            if (const auto* knowledge = beings_.raw().try_get<Knowledge>(h)) ecs::digest_component(*knowledge, d);
            if (const auto* history = beings_.raw().try_get<CraftHistory>(h)) ecs::digest_component(*history, d);
            if (const auto* lessons = beings_.raw().try_get<Lessons>(h)) ecs::digest_component(*lessons, d);
            if (const auto* env = beings_.raw().try_get<Habitat>(h)) ecs::digest_component(*env, d);
            if (const auto* thought = beings_.raw().try_get<Dream>(h)) ecs::digest_component(*thought, d);
            if (const auto* ledger = beings_.raw().try_get<Dreams>(h)) {
                d.i64(ledger->night);
                for (const auto sent : ledger->sent) d.u64(sent);
                d.u64(ledger->acts.size());
                for (const auto& act : ledger->acts) ecs::digest_component(act, d);
                d.u64(ledger->ended.size());
                for (const auto& ended : ledger->ended) {
                    d.u64(ended.id.value);
                    ecs::digest_component(ended.person, d);
                    d.i64(ended.ended_at);
                    d.u64(ended.next_memory);
                }
            }
            if (const auto* p = beings_.raw().try_get<Person>(h)) {
                d.u64(id.value);
                ecs::digest_component(*p, d);
            }
        });
        out.beings = d.value();
    }
    {
        num::Digest d;
        const auto each = [&](auto fn) {
            auto archived = archived_ids_.begin();
            things_.each([&](ecs::Id id, Things::Handle h) {
                while (archived != archived_ids_.end() && archived->first < id) {
                    fn(archived->first, std::optional<Things::Handle>{}, &item_archive_[archived->second]);
                    ++archived;
                }
                fn(id, std::optional<Things::Handle>{h}, static_cast<const ArchivedItem*>(nullptr));
            });
            while (archived != archived_ids_.end()) {
                fn(archived->first, std::optional<Things::Handle>{}, &item_archive_[archived->second]);
                ++archived;
            }
        };
        d.u64(things_.size() + item_archive_.size());
        each([&](ecs::Id id, auto h, const ArchivedItem* archived) {
            d.u64(id.value);
            d.u8(1);
            ecs::digest_component(ecs::Ident{id}, d);
            const auto* place = archived ? &archived->place : things_.raw().try_get<Place>(*h);
            d.u8(place ? 1 : 0);
            if (place) ecs::digest_component(*place, d);
        });
        each([&](ecs::Id id, auto h, const ArchivedItem* archived) {
            const auto* item = archived ? &archived->item : things_.raw().try_get<Item>(*h);
            if (item) {
                d.u64(id.value);
                ecs::digest_component(*item, d);
            }
            if (h)
                if (const auto* fire = things_.raw().try_get<Fire>(*h)) ecs::digest_component(*fire, d);
            const auto* timer =
                archived ? (archived->timer ? &*archived->timer : nullptr) : things_.raw().try_get<HeatTimer>(*h);
            if (timer) ecs::digest_component(*timer, d);
        });
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
