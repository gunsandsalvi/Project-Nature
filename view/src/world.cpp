#include "world.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <algorithm>
#include <optional>
#include <utility>
#include <vector>

#include "kd/core/check.hpp"
#include "kd/num/convert.hpp"
#include "kd/num/digest.hpp"
#include "kd/run/heat_tuning.hpp"
#include "kd/time/calendar.hpp"

namespace kd::view {

KdWorld::KdWorld() = default;

KdWorld::~KdWorld() {
    // the runner's thread stops before the world it runs goes
    runner_.reset();
}

void KdWorld::_bind_methods() {
    using godot::ClassDB;
    using godot::D_METHOD;
    ClassDB::bind_method(D_METHOD("load_catalogue", "paths"), &KdWorld::load_catalogue);
    ClassDB::bind_method(D_METHOD("entry", "folder", "name"), &KdWorld::entry);
    ClassDB::bind_method(D_METHOD("start_clockwork", "work_per_hour"), &KdWorld::start_clockwork);
    ClassDB::bind_method(D_METHOD("start_crowd", "seed", "camps"), &KdWorld::start_crowd);
    ClassDB::bind_method(D_METHOD("set_speed", "game_per_real"), &KdWorld::set_speed);
    ClassDB::bind_method(D_METHOD("speed"), &KdWorld::speed);
    ClassDB::bind_method(D_METHOD("pause"), &KdWorld::pause);
    ClassDB::bind_method(D_METHOD("play"), &KdWorld::play);
    ClassDB::bind_method(D_METHOD("is_paused"), &KdWorld::is_paused);
    ClassDB::bind_method(D_METHOD("frame"), &KdWorld::frame);
    ClassDB::bind_method(D_METHOD("screen_time"), &KdWorld::screen_time);
    ClassDB::bind_method(D_METHOD("frontier"), &KdWorld::frontier);
    ClassDB::bind_method(D_METHOD("speed_shown"), &KdWorld::speed_shown);
    ClassDB::bind_method(D_METHOD("date_text"), &KdWorld::date_text);
    ClassDB::bind_method(D_METHOD("time_text"), &KdWorld::time_text);
    ClassDB::bind_method(D_METHOD("counters"), &KdWorld::counters);
    ClassDB::bind_method(D_METHOD("drain_greetings"), &KdWorld::drain_greetings);
    ClassDB::bind_method(D_METHOD("set_pinned", "cores"), &KdWorld::set_pinned);
    ClassDB::bind_method(D_METHOD("heat_reading", "forecast"), &KdWorld::heat_reading);
    ClassDB::bind_method(D_METHOD("night_at", "t"), &KdWorld::night_at);
    ClassDB::bind_method(D_METHOD("crowd_square"), &KdWorld::crowd_square);
    ClassDB::bind_method(D_METHOD("run_until", "moment"), &KdWorld::run_until);
    ClassDB::bind_method(D_METHOD("begin_at", "moment"), &KdWorld::begin_at);
    ClassDB::bind_method(D_METHOD("places", "origin_east", "origin_north"), &KdWorld::places);
}

namespace {

godot::String text_of(const std::string& s) {
    return godot::String::utf8(s.c_str());
}

}  // namespace

godot::Dictionary KdWorld::load_catalogue(const godot::PackedStringArray& paths) {
    KD_CHECK(!runner_, "view::KdWorld: the catalogue is loaded before the world starts");
    const auto started = std::chrono::steady_clock::now();
    std::vector<data::SourceFile> files;
    godot::PackedStringArray problems;
    int64_t bytes = 0;
    for (int64_t i = 0; i < paths.size(); ++i) {
        const godot::String& path = paths[i];
        const godot::PackedByteArray b = godot::FileAccess::get_file_as_bytes("res://data/" + path);
        if (b.is_empty()) {
            problems.append(path + godot::String(": the file cannot be read"));
            continue;
        }
        files.push_back({path.utf8().get_data(), std::string(b.ptr(), b.ptr() + b.size())});
        bytes += b.size();
    }
    catalogue_ = std::make_unique<data::Catalogue>();
    for (const data::Problem& p : catalogue_->load(files)) {
        problems.append(text_of(data::problem_text(p)));
    }
    const auto took = std::chrono::steady_clock::now() - started;
    godot::Dictionary out;
    out["problems"] = problems;
    out["files"] = static_cast<int64_t>(files.size());
    out["bytes"] = bytes;
    out["microseconds"] = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::microseconds>(took).count());
    out["world_making_version"] = data::kWorldMakingVersion;
    godot::Array sources;
    for (const data::Source& s : catalogue_->sources()) {
        godot::Dictionary d;
        d["id"] = text_of(s.id);
        d["version"] = s.version;
        d["about"] = text_of(s.about);
        d["rules"] = text_of(num::to_hex(s.digests[0]));
        d["world"] = text_of(num::to_hex(s.digests[1]));
        d["look"] = text_of(num::to_hex(s.digests[2]));
        sources.append(d);
    }
    out["sources"] = sources;
    godot::Array kinds;
    for (const auto& k : catalogue_->kinds()) {
        godot::Dictionary d;
        d["folder"] = text_of(k->folder());
        d["about"] = text_of(k->about());
        godot::Array entries;
        for (std::size_t i = 0; i < k->size(); ++i) {
            godot::Dictionary e;
            e["name"] = text_of(k->name(i));
            e["file"] = text_of(k->file(i));
            e["values"] = text_of(k->display(i));
            entries.append(e);
        }
        d["entries"] = entries;
        kinds.append(d);
    }
    out["kinds"] = kinds;
    return out;
}

godot::Dictionary KdWorld::entry(const godot::String& folder, const godot::String& name) const {
    godot::Dictionary out;
    if (!catalogue_) {
        return out;
    }
    const std::string f = folder.utf8().get_data();
    const data::KindBase* k = catalogue_->kind_in(f);
    const std::optional<std::uint32_t> i = catalogue_->find(f, name.utf8().get_data());
    if (k == nullptr || !i) {
        return out;
    }
    for (const data::FieldValue& v : k->values(*i)) {
        const godot::String key = text_of(v.key);
        switch (v.kind) {
            case data::FieldValue::Kind::whole:
                out[key] = v.whole;
                break;
            case data::FieldValue::Kind::text:
                out[key] = text_of(v.text);
                break;
            case data::FieldValue::Kind::truth:
                out[key] = v.truth;
                break;
            case data::FieldValue::Kind::list: {
                godot::PackedStringArray list;
                for (const std::string& item : v.list) {
                    list.append(text_of(item));
                }
                out[key] = list;
                break;
            }
        }
    }
    return out;
}

void KdWorld::start_clockwork(int64_t work_per_hour) {
    KD_CHECK(!runner_, "view::KdWorld: the world has already started");
    KD_CHECK(work_per_hour >= 1, "view::KdWorld: the clockwork needs some work for each hour");
    clockwork_ = std::make_unique<demo::Clockwork>(static_cast<std::uint64_t>(work_per_hour));
    runner_ = std::make_unique<run::Runner>(*clockwork_, 0);
}

void KdWorld::start_crowd(int64_t seed, int64_t camps) {
    KD_CHECK(!runner_, "view::KdWorld: the world has already started");
    KD_CHECK(catalogue_ != nullptr, "view::KdWorld: the crowd needs the catalogue loaded first");
    KD_CHECK(seed >= 0 && camps >= 0, "view::KdWorld: a crowd's seed and camps are never negative");
    crowd_ = std::make_unique<demo::CrowdWorld>(static_cast<std::uint64_t>(seed), *catalogue_,
                                                camps > 0 ? std::optional<std::int64_t>(camps) : std::nullopt);
    stepper_ = std::make_unique<CrowdStepper>(*crowd_);
    heat_ = HeatGovernor(heat_rules());
    runner_ = std::make_unique<run::Runner>(*stepper_, 0, "kd-crowd");
}

HeatRules KdWorld::heat_rules() const {
    HeatRules rules;
    const std::optional<std::uint32_t> i = catalogue_->find("tuning/heat", "base:heat");
    if (!i) {
        return rules;
    }
    const run::HeatTuning& h = catalogue_->kind<run::HeatTuning>()[*i];
    const auto ratio = [](std::int64_t ppm) { return static_cast<double>(ppm) / 1.0e6; };
    rules.near = ratio(h.near);
    rules.cut = ratio(h.cut);
    rules.floor = ratio(h.floor);
    rules.calm_readings = static_cast<int>(h.calm / std::max<std::int64_t>(1, h.reading));
    rules.give_back = ratio(h.give_back);
    return rules;
}

void KdWorld::set_speed(double game_per_real) {
    pace_.set_speed(game_per_real);
}

double KdWorld::speed() const {
    return pace_.speed();
}

void KdWorld::pause() {
    pace_.pause();
}

void KdWorld::play() {
    pace_.play();
}

bool KdWorld::is_paused() const {
    return pace_.paused();
}

void KdWorld::frame() {
    if (!runner_) {
        return;
    }
    // each frame's own real time, from the steady clock: Godot's delta is smoothed and can hide a stall (research 18)
    const auto now = std::chrono::steady_clock::now();
    const double real = framed_ ? std::chrono::duration<double>(now - last_frame_).count() : 0.0;
    last_frame_ = now;
    framed_ = true;
    if (stepper_) {
        // the crowd is asked for no more than the phone can do, less what the heat holds back
        const double can = stepper_->capacity();
        if (can > 0.0) {
            pace_.set_limit(std::max(1.0, can * heat_.share() * kUse));
        }
    }
    runner_->set_goal(pace_.frame(real, runner_->frontier()));
    if (stepper_) {
        stepper_->set_screen(pace_.screen());
        real_ += real;
        event_samples_.emplace_back(real_, stepper_->events());
        while (event_samples_.size() > 2 && real_ - event_samples_[1].first >= 1.0) {
            event_samples_.pop_front();
        }
    }
}

double KdWorld::screen_time() const {
    return pace_.screen();
}

int64_t KdWorld::frontier() const {
    return runner_ ? runner_->frontier() : 0;
}

double KdWorld::speed_shown() const {
    return pace_.speed_shown();
}

godot::String KdWorld::date_text() const {
    const time::Date d = time::date_of(num::to_int(pace_.screen(), num::Round::down));
    return time::date_text(d).c_str();
}

godot::String KdWorld::time_text() const {
    const time::Date d = time::date_of(num::to_int(pace_.screen(), num::Round::down));
    return time::time_of_day_text(d).c_str();
}

godot::Dictionary KdWorld::counters() const {
    godot::Dictionary out;
    out["speed"] = pace_.speed();
    out["speed_shown"] = pace_.speed_shown();
    out["frontier"] = frontier();
    out["ahead"] = static_cast<double>(frontier()) - pace_.screen();
    if (!stepper_) {
        return out;
    }
    double per_second = 0.0;
    if (event_samples_.size() >= 2) {
        const auto& [t0, e0] = event_samples_.front();
        const auto& [t1, e1] = event_samples_.back();
        per_second = t1 > t0 ? static_cast<double>(e1 - e0) / (t1 - t0) : 0.0;
    }
    out["events_per_second"] = per_second;
    out["events"] = static_cast<int64_t>(stepper_->events());
    out["batches"] = static_cast<int64_t>(stepper_->batches());
    out["batch_ms"] = stepper_->last_batch_ms();
    out["greetings"] = static_cast<int64_t>(stepper_->greetings());
    out["capacity"] = stepper_->capacity();
    out["share"] = heat_.share();
    out["limit"] = pace_.limit();
    out["walkers"] = static_cast<int64_t>(stepper_->walker_count());
    return out;
}

godot::PackedInt64Array KdWorld::drain_greetings() {
    godot::PackedInt64Array out;
    if (!stepper_) {
        return out;
    }
    for (const Greeting& g : stepper_->drain_greetings()) {
        out.push_back(g.second);
        out.push_back(static_cast<int64_t>(g.from));
        out.push_back(static_cast<int64_t>(g.to));
    }
    return out;
}

void KdWorld::set_pinned(const godot::PackedInt32Array& cores) {
    if (!stepper_) {
        return;
    }
    std::vector<int> list;
    list.reserve(static_cast<std::size_t>(cores.size()));
    for (int64_t i = 0; i < cores.size(); ++i) {
        list.push_back(cores[i]);
    }
    stepper_->pin(std::move(list));
}

double KdWorld::heat_reading(double forecast) {
    return heat_.read(forecast);
}

bool KdWorld::night_at(double t) const {
    return crowd_ && crowd_->daylight().night_at(num::to_int(t, num::Round::down));
}

godot::Dictionary KdWorld::crowd_square() const {
    godot::Dictionary out;
    if (!crowd_) {
        return out;
    }
    const demo::Square s = crowd_->square();
    out["west"] = s.south_west.x;
    out["south"] = s.south_west.y;
    out["side"] = s.side;
    return out;
}

void KdWorld::run_until(int64_t moment) {
    KD_CHECK(runner_ != nullptr, "view::KdWorld: no world has started");
    runner_->set_goal(moment);
    runner_->wait_for(moment);
}

void KdWorld::begin_at(int64_t moment) {
    KD_CHECK(!framed_, "view::KdWorld: the screen begins before its first frame");
    run_until(moment);
    pace_ = Pace(static_cast<double>(moment));
    if (stepper_) {
        stepper_->set_screen(static_cast<double>(moment));
    }
}

godot::PackedFloat64Array KdWorld::places(int64_t origin_east, int64_t origin_north) const {
    godot::PackedFloat64Array out;
    if (!crowd_) {
        return out;
    }
    const world::World& w = crowd_->world();
    const num::Point origin = w.torus().wrap(origin_east, origin_north);
    w.beings().each([&](ecs::Id id, world::Beings::Handle h) {
        if (id.family() != ecs::Family::marker) {
            return;
        }
        const num::Offset o =
            w.torus().offset(origin, w.beings().raw().get<world::Activity>(h).at(w.torus(), w.frontier()));
        out.push_back(static_cast<double>(o.dx) / 100.0);
        out.push_back(static_cast<double>(o.dy) / 100.0);
    });
    return out;
}

}  // namespace kd::view
