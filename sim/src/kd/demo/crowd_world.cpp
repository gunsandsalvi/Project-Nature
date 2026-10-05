#include "kd/demo/crowd_world.hpp"

#include <algorithm>

#include "kd/chance/chance.hpp"
#include "kd/demo/marker.hpp"
#include "kd/demo/parts.hpp"
#include "kd/num/whole.hpp"

namespace kd::demo {

namespace {

const Crowd& crowd_of(const data::Catalogue& catalogue) {
    const data::Kind<Crowd>& kind = catalogue.kind<Crowd>();
    const std::optional<std::uint32_t> i = catalogue.find("tuning/crowd", "demo:crowd");
    KD_CHECK(i.has_value(), "demo::CrowdWorld: the catalogue has no demo:crowd tuning");
    return kind[*i];
}

time::Seconds time_of_day(time::Seconds t) {
    return num::floor_mod(t, time::kDay);
}

}  // namespace

Daylight::Daylight(world::World& w, time::Seconds dawn, time::Seconds dusk)
    : dawn_(dawn), dusk_(dusk), night_(night_at(w.frontier())) {
    KD_CHECK(dawn < dusk, "demo::Daylight: dawn comes before dusk");
    w.set_layer(ecs::owners::daylight, *this);
    w.schedule(ecs::owners::daylight, world::kActivitySlot, next_change(w.frontier()));
}

bool Daylight::night_at(time::Seconds t) const {
    const time::Seconds s = time_of_day(t);
    return s < dawn_ || s >= dusk_;
}

time::Seconds Daylight::next_change(time::Seconds t) const {
    const time::Seconds midnight = t - time_of_day(t);
    for (const time::Seconds candidate : {midnight + dawn_, midnight + dusk_, midnight + time::kDay + dawn_}) {
        if (candidate > t) {
            return candidate;
        }
    }
    return midnight + time::kDay + dawn_;
}

time::Seconds Daylight::next_dawn(time::Seconds t) const {
    const time::Seconds midnight = t - time_of_day(t);
    return midnight + dawn_ > t ? midnight + dawn_ : midnight + time::kDay + dawn_;
}

void Daylight::handle(world::World& w, const event::Event& /*e*/) {
    night_ = night_at(w.now());
    w.schedule(ecs::owners::daylight, world::kActivitySlot, next_change(w.now()));
}

void Daylight::digest(num::Digest& d) const {
    d.i64(dawn_);
    d.i64(dusk_);
    d.u8(night_ ? 1 : 0);
}

Markers::Markers(world::World& w, const Daylight& daylight, const Crowd& crowd, std::int64_t camps)
    : daylight_(daylight), kinds_(w.catalogue().kind<Marker>()), wander_(crowd.wander / 10) {
    KD_CHECK(kinds_.size() > 0, "demo::Markers: the catalogue has no markers");
    w.set_system(ecs::Family::marker, *this);
    const num::Torus& torus = w.torus();
    const std::int64_t half = crowd.area / 20;  // half the side, in centimetres
    const num::Point centre{torus.width() / 2, torus.height() / 2};
    const chance::Draws where(w.seed(), chance::name("markers"), 0, 0, chance::name("camps"));
    for (std::int64_t c = 0; c < camps; ++c) {
        const auto i = static_cast<std::uint64_t>(c);
        const num::Point at =
            torus.moved(centre, {where.between(2 * i, -half, half), where.between(2 * i + 1, -half, half)});
        const world::Beings::Handle camp = w.make_being(ecs::Family::place);
        w.beings().raw().emplace<world::Place>(camp, at);
        const ecs::Id camp_id = w.beings().id_of(camp);
        const chance::Draws kinds(w.seed(), chance::name("markers"), camp_id.value, 0, chance::name("kind"));
        for (std::int64_t m = 0; m < crowd.per_camp; ++m) {
            const auto j = static_cast<std::uint64_t>(m);
            const auto kind = static_cast<std::uint32_t>(kinds.below(j, kinds_.size()));
            const world::Beings::Handle h = w.make_being(ecs::Family::marker);
            const ecs::Id id = w.beings().id_of(h);
            auto& raw = w.beings().raw();
            raw.emplace<MarkerKind>(h, kind);
            raw.emplace<Home>(h, camp_id, at);
            // each first rests at its camp for a while, so the crowd does not all set off at once
            const chance::Draws first(w.seed(), chance::name("markers"), id.value, 0, chance::name("first rest"));
            const time::Seconds rest = std::max<time::Seconds>(1, kinds_[kind].rest.game);
            const time::Seconds end = w.frontier() + first.between(0, 1, rest);
            raw.emplace<world::Activity>(h, static_cast<std::uint8_t>(Doing::rest), w.frontier(), end, at, at);
            w.schedule(id, world::kActivitySlot, end);
        }
    }
}

void Markers::handle(world::World& w, const event::Event& e) {
    const ecs::Id id{e.key.owner};
    const std::optional<world::Beings::Handle> found = w.beings().find(id);
    KD_CHECK(found.has_value(), "demo::Markers: an event for a marker that is gone");
    auto& raw = w.beings().raw();
    world::Activity& a = raw.get<world::Activity>(*found);
    const Marker& kind = kinds_[raw.get<MarkerKind>(*found).kind];
    const num::Point here = a.to;
    const time::Seconds now = w.now();
    if (daylight_.night_at(now)) {
        a = {static_cast<std::uint8_t>(Doing::sleep), now, daylight_.next_dawn(now), here, here};
    } else if (a.what == static_cast<std::uint8_t>(Doing::walk)) {
        a = {static_cast<std::uint8_t>(Doing::rest), now, now + std::max<time::Seconds>(1, kind.rest.game), here, here};
    } else {
        // to a place chosen by keyed chance, up to the wander east or north of its camp
        const Home& home = raw.get<Home>(*found);
        const chance::Draws draws(w.seed(), chance::name("markers"), id.value, now, chance::name("destination"));
        const num::Point to =
            w.torus().moved(home.at, {draws.between(0, -wander_, wander_), draws.between(1, -wander_, wander_)});
        // millimetres over millimetres a second, rounded up to a whole second, and never less than one
        const std::int64_t way = w.torus().distance(here, to) * 10;
        const time::Seconds takes = std::max<time::Seconds>(1, (way + kind.speed - 1) / kind.speed);
        a = {static_cast<std::uint8_t>(Doing::walk), now, now + takes, here, to};
    }
    w.schedule(id, world::kActivitySlot, a.end);
}

CrowdWorld::CrowdWorld(std::uint64_t seed, const data::Catalogue& catalogue, std::optional<std::int64_t> camps)
    : crowd_(crowd_of(catalogue)),
      world_(seed, catalogue),
      daylight_(world_, crowd_.dawn, crowd_.dusk),
      markers_(world_, daylight_, crowd_, camps.value_or(crowd_.camps)) {}

}  // namespace kd::demo
