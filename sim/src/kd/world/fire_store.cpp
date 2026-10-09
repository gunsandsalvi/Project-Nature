#include "kd/world/fire_store.hpp"
#include <set>
#include "kd/world/craft_store.hpp"
namespace kd::world {
void save_fire(const World& w, std::vector<save::Chunk>& out) {
    if ((craft_features(w) & kFire) == 0) return;
    ByteWriter fires, thermal;
    fires.u64(w.things().raw().view<Fire>().size() + w.things().raw().view<HeatTimer>().size());
    w.things().each([&](ecs::Id id, Things::Handle h) {
        if (const auto* f = w.things().raw().try_get<Fire>(h)) {
            fires.u64(id.value);
            fires.u8(1);
            ecs::write_component(*f, fires);
        }
        if (const auto* t = w.things().raw().try_get<HeatTimer>(h)) {
            fires.u64(id.value);
            fires.u8(2);
            ecs::write_component(*t, fires);
        }
    });
    thermal.u64(w.beings().raw().view<Ambient>().size() + w.beings().raw().view<Thermal>().size());
    w.beings().each([&](ecs::Id id, Beings::Handle h) {
        if (const auto* a = w.beings().raw().try_get<Ambient>(h)) {
            thermal.u64(id.value);
            thermal.u8(1);
            ecs::write_component(*a, thermal);
        }
        if (const auto* t = w.beings().raw().try_get<Thermal>(h)) {
            thermal.u64(id.value);
            thermal.u8(2);
            ecs::write_component(*t, thermal);
        }
    });
    out.push_back({save::tag("FIRE"), 1, true, fires.take()});
    out.push_back({save::tag("THER"), 1, true, thermal.take()});
}
bool fire_headers(std::span<const save::Chunk> chunks, std::uint32_t features, std::string& why) {
    for (const auto tag : {save::tag("FIRE"), save::tag("THER")}) {
        const auto count = std::count_if(chunks.begin(), chunks.end(), [&](const auto& c) { return c.tag == tag; });
        const auto* c = save::find_chunk(chunks, tag);
        if (count > 1 || bool(c) != bool(features & kFire) || (c && (!c->critical || c->version != 1))) {
            why = "fire extension is missing, duplicated or mismatched";
            return false;
        }
    }
    return true;
}
bool load_fire(World& w, std::span<const save::Chunk> chunks, const ecs::EntryMap& entries, std::uint32_t features,
               std::string& why) {
    if ((features & kFire) == 0) return true;
    const auto fail = [&](std::string text) {
        why = std::move(text);
        return false;
    };
    const auto now = w.frontier();
    auto& things = w.things().raw();
    auto& beings = w.beings().raw();
    const auto deadline = [&](std::int64_t t) { return t == 0 || t >= now; };
    const auto camp = [&](ecs::Id id) {
        const auto h = w.beings().find(id);
        return h && beings.all_of<Camp>(*h);
    };
    {
        ByteReader r(save::find_chunk(chunks, save::tag("FIRE"))->data);
        std::uint64_t count = 0;
        std::pair<ecs::Id, std::uint8_t> last{};
        if (!r.u64(count) || count > w.things().size()) return fail("invalid fire record count");
        for (std::uint64_t n = 0; n < count; ++n) {
            ecs::Id id{};
            std::uint8_t kind = 0;
            if (!r.u64(id.value) || !r.u8(kind) || !(last < std::pair{id, kind}))
                return fail("invalid fire identity order");
            last = {id, kind};
            const auto h = w.things().find(id);
            if (!h || !things.all_of<Item, Place>(*h)) return fail("fire refers to missing physical item");
            if (things.any_of<Fire, HeatTimer>(*h)) return fail("mixed fire and food exposure identity");
            const auto& item = things.get<Item>(*h);
            if (kind == 1) {
                Fire f;
                if (!ecs::read_component(f, r, entries) || !camp(f.hearth) || item.home != f.hearth ||
                    f.at != things.get<Place>(*h).at || f.heat > 5 || f.ring > 1 || f.unblown_checked > 1 ||
                    f.damp_remainder < 0 || f.damp_remainder >= 1000000 || f.evaporated_mg < 0 || f.fuel_mg < 0 ||
                    f.ash_mg < 0 || f.fuel_mg > item.mass || f.ash_mg != item.mass - f.fuel_mg ||
                    f.burn_remainder < 0 || f.burn_remainder >= time::kHour || f.settled_at < 0 || f.settled_at > now ||
                    !deadline(f.next) || f.embers_until < 0 || f.banked_until < 0 || f.air_until < 0 ||
                    (f.heat >= 2 && f.fuel_mg == 0))
                    return fail("invalid conserved fire state");
                if (f.owner.value) {
                    const auto p = w.beings().find(f.owner);
                    if (!p || !beings.all_of<Person>(*p) || item.owner != f.owner) return fail("invalid fire carrier");
                }
                things.emplace<Fire>(*h, f);
            } else if (kind == 2) {
                HeatTimer t;
                if (!ecs::read_component(t, r, entries) || t.item != id || t.target_state != 1 || t.low != 2 ||
                    t.high != 3 || t.completed > 1 || t.tried > 1 || t.intended > 1 || t.elapsed < 0 ||
                    t.elapsed > 2 * time::kHour || t.hot_elapsed < 0 || t.hot_elapsed > time::kHour ||
                    t.hot_elapsed > t.elapsed || t.settled_at < 0 || t.settled_at > now || !deadline(t.next) ||
                    (t.completed && (t.next != 0 || item.state != 2)))
                    return fail("invalid retained heat exposure");
                if (t.maker.value) {
                    const auto maker = w.beings().find(t.maker);
                    if (!maker || !beings.all_of<Person>(*maker)) return fail("missing cooking placer");
                } else if (t.intended)
                    return fail("intentional cooking has no placer");
                things.emplace<HeatTimer>(*h, t);
            } else
                return fail("unknown fire record kind");
        }
        if (!r.finished()) return fail("trailing fire records");
    }
    {
        ByteReader r(save::find_chunk(chunks, save::tag("THER"))->data);
        std::uint64_t count = 0;
        ecs::Id last{};
        const auto expected = beings.view<Camp>().size() + beings.view<Person>().size();
        if (!r.u64(count) || count != expected) return fail("thermal records do not cover this camp");
        for (std::uint64_t n = 0; n < count; ++n) {
            ecs::Id id{};
            std::uint8_t kind = 0;
            if (!r.u64(id.value) || !(last < id) || !r.u8(kind)) return fail("invalid thermal identity order");
            last = id;
            const auto h = w.beings().find(id);
            if (!h) return fail("missing thermal owner");
            if (kind == 1) {
                Ambient a;
                if (!beings.all_of<Camp>(*h) || !ecs::read_component(a, r, entries) ||
                    (a.milli_c != 18000 && a.milli_c != 24000) || a.next <= 0 || !deadline(a.next))
                    return fail("invalid mild camp ambient");
                beings.emplace<Ambient>(*h, a);
            } else if (kind == 2) {
                Thermal t;
                if (!beings.all_of<Person>(*h) || !ecs::read_component(t, r, entries) || t.felt_milli_c < 18000 ||
                    t.felt_milli_c > 39000 || t.warmth < 0 || t.warmth > 100 || t.settled_at < 0 ||
                    t.settled_at > now || t.warming_progress < 0 || t.warming_progress > now || t.water_remainder < 0 ||
                    t.water_remainder >= time::kDay * 100000 || t.water_used_ml < 0)
                    return fail("invalid felt temperature or thermal remainder");
                beings.emplace<Thermal>(*h, t);
            } else
                return fail("unknown thermal record kind");
        }
        if (!r.finished()) return fail("trailing thermal records");
    }
    // These deadlines are saved camp events, not frame polling. Reopen must not
    // repair a lost event.
    bool valid = true;
    const auto events = w.queue().live_in_order([&](const auto& e) { return w.live(e); });
    w.beings().each([&](ecs::Id id, Beings::Handle h) {
        const auto* a = beings.try_get<Ambient>(h);
        if (!a) return;
        auto heat = a->next;
        std::int64_t timer = 0;
        w.things().each([&](ecs::Id, Things::Handle th) {
            if (const auto* f = things.try_get<Fire>(th); f && f->hearth == id && f->next)
                heat = std::min(heat, f->next);
            if (const auto* t = things.try_get<HeatTimer>(th); t && things.get<Item>(th).home == id && t->next)
                timer = timer ? std::min(timer, t->next) : t->next;
        });
        std::size_t heat_count = 0, timer_count = 0;
        for (const auto& e : events) {
            if (e.key.owner != id.value) continue;
            if (e.slot == 2) {
                ++heat_count;
                if (e.key.second != heat) valid = false;
            }
            if (e.slot == 3) {
                ++timer_count;
                if (e.key.second != timer) valid = false;
            }
        }
        if (heat_count != 1 || timer_count != static_cast<std::size_t>(timer != 0)) valid = false;
    });
    return valid || fail("fire or thermal transition has no matching live event");
}
}  // namespace kd::world
