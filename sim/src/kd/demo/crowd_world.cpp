#include "kd/demo/crowd_world.hpp"

#include <algorithm>

#include "kd/chance/chance.hpp"
#include "kd/demo/parts.hpp"
#include "kd/num/whole.hpp"

namespace kd::demo {

namespace {

// The side of the grid's cells, 250 m, in centimetres.
constexpr std::int64_t kCell = 25'000;
// Beyond the camps' square and the wander, room for anything rounding pushes out.
constexpr std::int64_t kMargin = 100'000;

const Crowd& crowd_of(const data::Catalogue& catalogue) {
    const data::Kind<Crowd>& kind = catalogue.kind<Crowd>();
    const std::optional<std::uint32_t> i = catalogue.find("tuning/crowd", "demo:crowd");
    KD_CHECK(i.has_value(), "demo::CrowdWorld: the catalogue has no demo:crowd tuning");
    return kind[*i];
}

time::Seconds time_of_day(time::Seconds t) {
    return num::floor_mod(t, time::kDay);
}

void sort_unique(std::vector<ecs::Id>& ids) {
    std::stable_sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
}

// Half the side of the square the crowd can be in: the camps' half-square, the wander and a margin.
std::int64_t half_side(const Crowd& crowd) {
    return crowd.area / 20 + crowd.wander / 10 + kMargin;
}

num::Point centre_of(const num::Torus& torus) {
    return {torus.width() / 2, torus.height() / 2};
}

std::uint8_t doing(Doing d) {
    return static_cast<std::uint8_t>(d);
}

}  // namespace

// --- Daylight

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
    for (const time::Seconds candidate : {midnight + dawn_, midnight + dusk_}) {
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

void Daylight::handle(world::Context& c, const event::Event& /*e*/) {
    night_ = night_at(c.now());
    c.schedule(ecs::owners::daylight, world::kActivitySlot, next_change(c.now()));
}

void Daylight::digest(num::Digest& d) const {
    d.i64(dawn_);
    d.i64(dusk_);
    d.u8(night_ ? 1 : 0);
}

// --- Grid

Grid::Grid(const num::Torus& torus, num::Point south_west, std::int64_t side, std::int64_t cell)
    : torus_(torus), south_west_(south_west), cell_(cell), count_((side + cell - 1) / cell) {
    lists_.resize(static_cast<std::size_t>(count_ * count_));
}

std::int64_t Grid::clamp_cell(std::int64_t v) const {
    return std::clamp<std::int64_t>(num::floor_div(v, cell_), 0, count_ - 1);
}

Grid::Cells Grid::cells(num::Point from, num::Point to, std::int64_t margin) const {
    const num::Offset f = torus_.offset(south_west_, from);
    const num::Offset t = torus_.offset(south_west_, to);
    return {clamp_cell(std::min(f.dx, t.dx) - margin), clamp_cell(std::min(f.dy, t.dy) - margin),
            clamp_cell(std::max(f.dx, t.dx) + margin), clamp_cell(std::max(f.dy, t.dy) + margin)};
}

void Grid::put(ecs::Id id, const Cells& c) {
    for (std::int64_t y = c.y0; y <= c.y1; ++y) {
        for (std::int64_t x = c.x0; x <= c.x1; ++x) {
            lists_[static_cast<std::size_t>(y * count_ + x)].push_back(id);
        }
    }
}

void Grid::take(ecs::Id id, const Cells& c) {
    for (std::int64_t y = c.y0; y <= c.y1; ++y) {
        for (std::int64_t x = c.x0; x <= c.x1; ++x) {
            std::vector<ecs::Id>& list = lists_[static_cast<std::size_t>(y * count_ + x)];
            const auto at = std::find(list.begin(), list.end(), id);
            KD_CHECK(at != list.end(), "demo::Grid: a marker missing from its cell");
            *at = list.back();
            list.pop_back();
        }
    }
}

void Grid::collect(const Cells& c, std::vector<ecs::Id>& out) const {
    for (std::int64_t y = c.y0; y <= c.y1; ++y) {
        for (std::int64_t x = c.x0; x <= c.x1; ++x) {
            const std::vector<ecs::Id>& list = lists_[static_cast<std::size_t>(y * count_ + x)];
            out.insert(out.end(), list.begin(), list.end());
        }
    }
}

// --- Markers

Markers::Markers(world::World& w, const Daylight& daylight, const Crowd& crowd, std::int64_t camps)
    : daylight_(daylight),
      kinds_(w.catalogue().kind<Marker>()),
      wander_(crowd.wander / 10),
      homeward_(crowd.homeward),
      greeting_(std::max<time::Seconds>(1, crowd.greeting.game)),
      grid_(w.torus(), w.torus().moved(centre_of(w.torus()), {-half_side(crowd), -half_side(crowd)}),
            2 * half_side(crowd), kCell) {
    KD_CHECK(kinds_.size() > 0, "demo::Markers: the catalogue has no markers");
    for (std::size_t k = 0; k < kinds_.size(); ++k) {
        reach_ = std::max(reach_, kinds_[static_cast<std::uint32_t>(k)].reach / 10 + 1);
    }
    w.set_system(ecs::Family::marker, *this);
    const num::Torus& torus = w.torus();
    const std::int64_t half = crowd.area / 20;  // half the side, in centimetres
    const num::Point centre = centre_of(torus);
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
            raw.emplace<world::Activity>(h, doing(Doing::rest), w.frontier(), end, at, at);
            w.schedule(id, world::kActivitySlot, end);
            ids_.push_back(id);
            cells_.push_back(grid_.cells(at, at, 0));
            grid_.put(id, cells_.back());
        }
    }
}

std::size_t Markers::index_of(ecs::Id id) const {
    const auto at = std::lower_bound(ids_.begin(), ids_.end(), id);
    KD_CHECK(at != ids_.end() && *at == id, "demo::Markers: no such marker");
    return static_cast<std::size_t>(at - ids_.begin());
}

void Markers::regrid(const world::World& w, ecs::Id id) {
    const std::size_t i = index_of(id);
    const world::Activity& a = w.beings().raw().get<world::Activity>(w.beings().handle(id));
    const Grid::Cells now = grid_.cells(a.from, a.to, 0);
    const Grid::Cells& was = cells_[i];
    if (now.x0 == was.x0 && now.y0 == was.y0 && now.x1 == was.x1 && now.y1 == was.y1) {
        return;
    }
    grid_.take(id, was);
    grid_.put(id, now);
    cells_[i] = now;
}

void Markers::moved(world::Context& c, ecs::Id id) {
    if (c.island()) {
        c.moved(id);
    } else {
        regrid(c.world(), id);
    }
}

void Markers::after_window(world::World& w, std::span<const ecs::Id> moved) {
    for (const ecs::Id id : moved) {
        if (id.family() == ecs::Family::marker) {
            regrid(w, id);
        }
    }
}

void Markers::handle(world::Context& c, const event::Event& e) {
    const ecs::Id id{e.key.owner};
    world::World& w = c.world();
    const std::optional<world::Beings::Handle> found = w.beings().find(id);
    KD_CHECK(found.has_value(), "demo::Markers: an event for a marker that is gone");
    if (e.slot == world::kCallSlot) {
        called(c, *found, id);
        return;
    }
    auto& raw = w.beings().raw();
    world::Activity& a = raw.get<world::Activity>(*found);
    const Marker& kind = kinds_[raw.get<MarkerKind>(*found).kind];
    const num::Point here = a.to;
    const time::Seconds now = c.now();
    if (daylight_.night_at(now)) {
        a = {doing(Doing::sleep), now, daylight_.next_dawn(now), here, here};
    } else if (a.what == doing(Doing::walk)) {
        walk_ended(c, *found, id);
        return;
    } else if (a.what == doing(Doing::greet)) {
        a = {doing(Doing::rest), now, now + std::max<time::Seconds>(1, kind.rest.game), here, here};
    } else {
        // to its camp, where others gather, or to a place chosen by keyed chance up to the wander east or north of it
        const Home& home = raw.get<Home>(*found);
        const chance::Draws draws(w.seed(), chance::name("markers"), id.value, now, chance::name("destination"));
        const num::Point to =
            draws.fires(0, homeward_)
                ? home.at
                : w.torus().moved(home.at, {draws.between(1, -wander_, wander_), draws.between(2, -wander_, wander_)});
        // millimetres over millimetres a second, rounded up to a whole second, and never less than one
        const std::int64_t way = w.torus().distance(here, to) * 10;
        const time::Seconds takes = std::max<time::Seconds>(1, (way + kind.speed - 1) / kind.speed);
        a = {doing(Doing::walk), now, now + takes, here, to};
    }
    c.schedule(id, world::kActivitySlot, a.end);
    moved(c, id);
}

void Markers::walk_ended(world::Context& c, world::Beings::Handle h, ecs::Id id) {
    world::World& w = c.world();
    auto& raw = w.beings().raw();
    world::Activity& a = raw.get<world::Activity>(h);
    const Marker& kind = kinds_[raw.get<MarkerKind>(h).kind];
    const num::Point here = a.to;
    const time::Seconds now = c.now();
    // it may greet one of those within reach, who stops what it is doing a second later, when the call lands
    const chance::Draws draws(w.seed(), chance::name("markers"), id.value, now, chance::name("greeting"));
    if (draws.fires(0, kind.greets)) {
        const std::vector<ecs::Id> others = greetable(c, id, here, now, kind.reach / 10);
        if (!others.empty()) {
            const ecs::Id other = others[draws.below(1, others.size())];
            c.schedule(other, world::kCallSlot, now + 1);
            c.record(static_cast<std::uint32_t>(Happened::greeting), id.value, other.value);
            a = {doing(Doing::greet), now, now + 1 + greeting_, here, here};
            c.schedule(id, world::kActivitySlot, a.end);
            moved(c, id);
            return;
        }
    }
    a = {doing(Doing::rest), now, now + std::max<time::Seconds>(1, kind.rest.game), here, here};
    c.schedule(id, world::kActivitySlot, a.end);
    moved(c, id);
}

void Markers::called(world::Context& c, world::Beings::Handle h, ecs::Id id) {
    world::World& w = c.world();
    world::Activity& a = w.beings().raw().get<world::Activity>(h);
    // asleep, or already greeting another, it lets the call pass
    if (a.what == doing(Doing::sleep) || a.what == doing(Doing::greet)) {
        return;
    }
    const time::Seconds now = c.now();
    // what it was doing ends here, keeping what it reached: a walker stands where it got to (TIM-17)
    a.cut(w.torus(), now);
    c.cancel(id, world::kActivitySlot);
    a = {doing(Doing::greet), now, now + greeting_, a.to, a.to};
    c.schedule(id, world::kActivitySlot, a.end);
    moved(c, id);
}

std::vector<ecs::Id> Markers::greetable(world::Context& c, ecs::Id self, num::Point at, time::Seconds t,
                                        std::int64_t reach) const {
    const world::World& w = c.world();
    std::vector<ecs::Id> found;
    grid_.collect(grid_.cells(at, at, reach), found);
    // in an island, the grid is as the window began, so those that moved since are looked at too
    const std::span<const ecs::Id> moved = c.moved();
    found.insert(found.end(), moved.begin(), moved.end());
    sort_unique(found);
    std::vector<ecs::Id> out;
    for (const ecs::Id other : found) {
        if (other == self || other.family() != ecs::Family::marker || !c.holds(other)) {
            continue;
        }
        c.touch(other);
        const world::Beings::Handle h = w.beings().handle(other);
        const world::Activity& a = w.beings().raw().get<world::Activity>(h);
        if (a.what == doing(Doing::sleep) || a.what == doing(Doing::greet) ||
            w.beings().raw().get<world::Schedule>(h).expected[world::kCallSlot] != 0) {
            continue;
        }
        if (w.torus().squared_distance(a.at(w.torus(), t), at) <= reach * reach) {
            out.push_back(other);
        }
    }
    return out;
}

time::Seconds Markers::next_walk(const world::Activity& a, const Marker& kind, time::Seconds start) const {
    const time::Seconds rest = std::max<time::Seconds>(1, kind.rest.game);
    time::Seconds next = a.end;
    if (a.what == doing(Doing::walk) || a.what == doing(Doing::greet)) {
        next = daylight_.night_at(a.end) ? daylight_.next_dawn(a.end) : a.end + rest;
    } else if (a.what == doing(Doing::rest)) {
        next = daylight_.night_at(a.end) ? daylight_.next_dawn(a.end) : a.end;
    }
    // a call can cut it short at any second: then it greets and rests, or sleeps till dawn, before walking again
    if (a.what != doing(Doing::sleep)) {
        next = std::min(next, start + greeting_ + rest);
    }
    return next;
}

void Markers::bounds(const world::World& w, time::Seconds a, time::Seconds b, std::span<const ecs::Id> owners,
                     std::vector<world::Bound>& out) const {
    const num::Torus& torus = w.torus();
    const auto& raw = w.beings().raw();
    for (const ecs::Id id : owners) {
        const world::Beings::Handle h = w.beings().handle(id);
        const world::Activity& act = raw.get<world::Activity>(h);
        const Marker& kind = kinds_[raw.get<MarkerKind>(h).kind];
        const num::Point centre = act.at(torus, a);
        // along its way until the window ends, then as far as it can walk from when it can set off again
        std::int64_t radius = 1;
        if (act.what == doing(Doing::walk)) {
            radius += torus.distance(centre, act.at(torus, std::min(act.end, b))) + 1;
        }
        const time::Seconds sets_off = next_walk(act, kind, a);
        if (sets_off < b) {
            radius += (kind.speed + 9) / 10 * (b - sets_off);
        }
        out.push_back({id, centre, radius});
    }
}

void Markers::near(const world::World& /*w*/, time::Seconds /*a*/, time::Seconds /*b*/, const world::Bound& bound,
                   std::vector<ecs::Id>& out) const {
    // a marker with no events in the window stays on its present way, which the grid lists it under, once in each
    // cell the way crosses
    const std::size_t from = out.size();
    grid_.collect(grid_.cells(bound.centre, bound.centre, bound.radius + reach_), out);
    std::stable_sort(out.begin() + static_cast<std::ptrdiff_t>(from), out.end());
    out.erase(std::unique(out.begin() + static_cast<std::ptrdiff_t>(from), out.end()), out.end());
}

CrowdWorld::CrowdWorld(std::uint64_t seed, const data::Catalogue& catalogue, std::optional<std::int64_t> camps)
    : crowd_(crowd_of(catalogue)),
      world_(seed, catalogue),
      daylight_(world_, crowd_.dawn, crowd_.dusk),
      markers_(world_, daylight_, crowd_, camps.value_or(crowd_.camps)) {}

}  // namespace kd::demo
