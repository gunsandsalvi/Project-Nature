#include "world.hpp"
#include <limits>

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <sys/statvfs.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <optional>
#include <system_error>
#include <utility>
#include <vector>

#include "kd/core/check.hpp"
#include "kd/demo/kept.hpp"
#include "kd/look/navigation.hpp"
#include "kd/look/sprite.hpp"
#include "kd/look/stream_tuning.hpp"
#include "kd/num/convert.hpp"
#include "kd/num/digest.hpp"
#include "kd/run/heat_tuning.hpp"
#include "kd/run/save_tuning.hpp"
#include "kd/time/calendar.hpp"
#include "kd/time/speeds.hpp"
#include "trace.hpp"

namespace kd::view {

KdWorld::KdWorld() = default;

KdWorld::~KdWorld() {
    // the runner's thread stops before the world it runs goes, and the keeper writes what it was given
    runner_.reset();
    if (keeper_) {
        keeper_->flush();
    }
}

void KdWorld::_bind_methods() {
    using godot::ClassDB;
    using godot::D_METHOD;
    ClassDB::bind_method(D_METHOD("load_catalogue", "paths"), &KdWorld::load_catalogue);
    ClassDB::bind_method(D_METHOD("entry", "folder", "name"), &KdWorld::entry);
    ClassDB::bind_method(D_METHOD("sprite_families"), &KdWorld::sprite_families);
    ClassDB::bind_method(D_METHOD("stream_limits"), &KdWorld::stream_limits);
    ClassDB::bind_method(D_METHOD("start_clockwork"), &KdWorld::start_clockwork);
    ClassDB::bind_method(D_METHOD("start_crowd", "seed", "camps"), &KdWorld::start_crowd);
    ClassDB::bind_method(D_METHOD("open_crowd", "folder", "seed", "camps", "build"), &KdWorld::open_crowd);
    ClassDB::bind_method(D_METHOD("open_camp", "folder", "seed", "build"), &KdWorld::open_camp);
    ClassDB::bind_method(D_METHOD("people"), &KdWorld::people);
    ClassDB::bind_method(D_METHOD("camp_alpha"), &KdWorld::camp_alpha);
    ClassDB::bind_method(D_METHOD("save"), &KdWorld::save);
    ClassDB::bind_method(D_METHOD("save_now"), &KdWorld::save_now);
    ClassDB::bind_method(D_METHOD("call_home", "camp"), &KdWorld::call_home);
    ClassDB::bind_method(D_METHOD("nearest_camp", "east", "north", "within"), &KdWorld::nearest_camp);
    ClassDB::bind_method(D_METHOD("camp_at", "camp"), &KdWorld::camp_at);
    ClassDB::bind_method(D_METHOD("catching_up"), &KdWorld::catching_up);
    ClassDB::bind_method(D_METHOD("digest"), &KdWorld::digest);
    ClassDB::bind_method(D_METHOD("mark", "second"), &KdWorld::mark);
    ClassDB::bind_method(D_METHOD("marks"), &KdWorld::marks);
    ClassDB::bind_method(D_METHOD("call_home_at", "camp", "second"), &KdWorld::call_home_at);
    ClassDB::bind_method(D_METHOD("reach", "moment"), &KdWorld::reach);
    ClassDB::bind_static_method("KdWorld", D_METHOD("moment_text", "second"), &KdWorld::moment_text);
    ClassDB::bind_static_method("KdWorld", D_METHOD("crowd_seed"), &KdWorld::crowd_seed);
    ClassDB::bind_static_method("KdWorld", D_METHOD("morning"), &KdWorld::morning);
    ClassDB::bind_method(D_METHOD("navigation_tuning"), &KdWorld::navigation_tuning);
    ClassDB::bind_method(D_METHOD("enable_time_requests", "enabled"), &KdWorld::enable_time_requests);
    ClassDB::bind_method(D_METHOD("set_zoom_density", "density"), &KdWorld::set_zoom_density);
    ClassDB::bind_method(D_METHOD("set_manual_rate", "rate"), &KdWorld::set_manual_rate);
    ClassDB::bind_method(D_METHOD("clear_manual_rate"), &KdWorld::clear_manual_rate);
    ClassDB::bind_method(D_METHOD("set_speed_lock", "locked"), &KdWorld::set_speed_lock);
    ClassDB::bind_method(D_METHOD("time_requests"), &KdWorld::time_requests);
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
    ClassDB::bind_method(D_METHOD("set_heat_light", "threshold"), &KdWorld::set_heat_light);
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

// Read the schema itself: the binding never maintains a second list of family or cell fields.
struct SpriteFields {
    godot::Dictionary values;
    void whole(const data::Field& f, std::int64_t value, data::Range) { values[text_of(std::string(f.key))] = value; }
    void quantity(const data::Field& f, std::int64_t value, data::Measure, data::Range range) {
        whole(f, value, range);
    }
    void text(const data::Field& f, const std::string& value) { values[text_of(std::string(f.key))] = text_of(value); }
    void link(const data::Field& f, const data::Ref& value, std::string_view) { text(f, value.name); }
    template <typename T>
    void records(const data::Field& f, const std::vector<T>& value) {
        godot::Array rows;
        for (const auto& record : value) {
            SpriteFields fields;
            T::visit(fields, record);
            rows.push_back(fields.values);
        }
        values[text_of(std::string(f.key))] = rows;
    }
};

}  // namespace

godot::Array KdWorld::sprite_families() const {
    godot::Array rows;
    if (!catalogue_) {
        return rows;
    }
    const auto& families = catalogue_->kind<look::SpriteFamily>();
    for (std::uint32_t i = 0; i < families.size(); ++i) {
        SpriteFields fields;
        look::SpriteFamily::visit(fields, families[i]);
        fields.values["id"] = text_of(families.name(i));
        rows.push_back(fields.values);
    }
    return rows;
}

godot::Dictionary KdWorld::stream_limits() const {
    if (!catalogue_) return {};
    const auto& tuning = catalogue_->kind<look::StreamTuning>();
    const auto index = catalogue_->find("tuning/stream", "base:stream");
    if (!index) return {};
    const auto& s = tuning[*index];
    SpriteFields fields;
    look::StreamTuning::visit(fields, s);
    godot::Dictionary categories;
    categories["maps"] = s.maps_bytes;
    categories["sprites"] = s.sprites_bytes;
    categories["ground"] = s.ground_bytes;
    categories["masks"] = s.masks_bytes;
    fields.values["resident_by_category"] = categories;
    return fields.values;
}

godot::Dictionary KdWorld::navigation_tuning() const {
    if (!catalogue_) return {};
    const auto index = catalogue_->find("tuning/navigation", "base:navigation");
    if (!index) return {};
    SpriteFields fields;
    look::NavigationTuning::visit(fields, catalogue_->kind<look::NavigationTuning>()[*index]);
    return fields.values;
}
// T2.9a.3: opt-in request resolution leaves the accepted legacy Pace API intact (TIM-01/TIM-15).
void KdWorld::enable_time_requests(bool enabled) {
    time_requests_enabled_ = false;
    if (!enabled || !catalogue_) return;
    const auto navigation = catalogue_->find("tuning/navigation", "base:navigation");
    const auto time = catalogue_->find("tuning/time", "base:time");
    if (!navigation || !time) return;
    const auto& n = catalogue_->kind<look::NavigationTuning>()[*navigation];
    const auto& t = catalogue_->kind<time::ZoomSpeeds>()[*time];
    time_requests_enabled_ = time_requests_.configure({{std::ldexp(1.0, n.person_power), t.person / 60.0},
                                                       {std::ldexp(1.0, n.close_camp_power), t.close_camp / 60.0},
                                                       {std::ldexp(1.0, n.camp_power), t.camp / 60.0},
                                                       {std::ldexp(1.0, n.valley_power), t.valley / 60.0},
                                                       {std::ldexp(1.0, n.region_power), t.region / 60.0}},
                                                      std::ldexp(1.0, n.minimum_power));
}
void KdWorld::set_zoom_density(double density) {
    time_requests_.zoom(density);
}
void KdWorld::set_manual_rate(double rate) {
    // A game goal must still fit its signed whole-second frontier even before capacity is measured.
    if (std::isfinite(rate) && rate >= 1.0 &&
        rate <= (static_cast<double>(std::numeric_limits<std::int64_t>::max()) - pace_.screen()) / 4.0)
        time_requests_.manual(rate);
}
void KdWorld::clear_manual_rate() {
    time_requests_.clear_manual();
}
void KdWorld::set_speed_lock(bool locked) {
    time_requests_.lock(locked);
}
godot::Dictionary KdWorld::time_requests() const {
    godot::Dictionary result;
    const auto request = time_requests_.resolve();
    result["enabled"] = time_requests_enabled_;
    result["requested_rate"] = request.rate;
    result["source"] = request.source;
    result["zoom_rate"] = time_requests_.zoom_rate();
    result["density"] = time_requests_.density();
    result["locked"] = time_requests_.locked();
    result["paused"] = pace_.paused();
    result["capacity"] = pace_.limit() < 1e299 ? pace_.limit() : 0.0;
    return result;
}

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

void KdWorld::start_clockwork() {
    KD_CHECK(!runner_, "view::KdWorld: the world has already started");
    clockwork_ = std::make_unique<demo::Clockwork>(demo::kCalendarWork);
    start_runner(*clockwork_, [this] { return clockwork_->state(); }, 0, "kd-world");
}

void KdWorld::start_runner(run::Steppable& world, std::function<std::uint64_t()> digest, time::Seconds start,
                           const char* thread) {
    marked_ = std::make_unique<run::Marked>(world, std::move(digest));
    runner_ = std::make_unique<run::Runner>(*marked_, start, thread);
    if (stepper_) {
        display_.acquire(*stepper_);
    }
}

void KdWorld::start_crowd(int64_t seed, int64_t camps) {
    KD_CHECK(!runner_, "view::KdWorld: the world has already started");
    KD_CHECK(catalogue_ != nullptr, "view::KdWorld: the crowd needs the catalogue loaded first");
    KD_CHECK(seed >= 0 && camps >= 0, "view::KdWorld: a crowd's seed and camps are never negative");
    crowd_ = std::make_unique<demo::CrowdWorld>(static_cast<std::uint64_t>(seed), *catalogue_,
                                                camps > 0 ? std::optional<std::int64_t>(camps) : std::nullopt);
    stepper_ = std::make_unique<CrowdStepper>(*crowd_);
    heat_ = HeatGovernor(heat_rules());
    start_runner(*stepper_, [this] { return crowd_->world().digests().whole; }, 0, "kd-crowd");
}

godot::Dictionary KdWorld::open_crowd(const godot::String& folder, int64_t seed, int64_t camps,
                                      const godot::String& build) {
    return open_saved(folder, seed, camps, build, false);
}

godot::Dictionary KdWorld::open_camp(const godot::String& folder, int64_t seed, const godot::String& build) {
    return open_saved(folder, seed, 1, build, true);
}

godot::Dictionary KdWorld::open_saved(const godot::String& folder, int64_t seed, int64_t camps,
                                      const godot::String& build, bool camp_alpha) {
    KD_CHECK(!runner_, "view::KdWorld: the world has already started");
    KD_CHECK(catalogue_ != nullptr, "view::KdWorld: the crowd needs the catalogue loaded first");
    KD_CHECK(seed >= 0 && camps >= 0, "view::KdWorld: a crowd's seed and camps are never negative");
    godot::Dictionary out;
    const std::string path = folder.utf8().get_data();
    std::error_code made_folder;
    std::filesystem::create_directories(path, made_folder);
    if (made_folder) {
        out["problem"] = godot::String("the world's folder cannot be made: ") + folder;
        return out;
    }
    folder_ = path;
    files_ = std::make_unique<save::DiskFiles>(path);
    keeper_ = std::make_unique<save::Keeper>(*files_, build.utf8().get_data());
    demo::Kept kept = demo::keep_crowd(*keeper_, *catalogue_, static_cast<std::uint64_t>(seed), camps,
                                       world::migrations(), camp_alpha);
    godot::PackedStringArray damaged;
    for (const std::string& d : kept.damaged) {
        damaged.append(text_of(d));
    }
    out["damaged"] = damaged;
    const std::array<const char*, 3> updates{"none", "small", "big"};
    out["update"] = updates.at(static_cast<std::size_t>(kept.update));
    godot::PackedStringArray migrated;
    for (const std::string& m : kept.migrated) {
        migrated.append(text_of(m));
    }
    out["migrated"] = migrated;
    if (!kept.crowd || !kept.problem.empty()) {
        out["problem"] = text_of(kept.problem);
        return out;
    }
    crowd_ = std::move(kept.crowd);
    stepper_ = std::make_unique<CrowdStepper>(*crowd_);
    stepper_->keep(keeper_.get());
    heat_ = HeatGovernor(heat_rules());
    const time::Seconds frontier = crowd_->world().frontier();
    pace_ = Pace(static_cast<double>(frontier));
    stepper_->set_screen(static_cast<double>(frontier));
    start_runner(*stepper_, [this] { return crowd_->world().digests().whole; }, frontier, "kd-crowd");
    if (kept.was_at > frontier) {
        catch_up_to_ = kept.was_at;
        runner_->set_goal(kept.was_at);
    }
    if (const std::optional<std::uint32_t> i = catalogue_->find("tuning/saves", "base:saves")) {
        save_every_ = static_cast<double>(catalogue_->kind<run::SaveTuning>()[*i].every);
        warn_below_mb_ = catalogue_->kind<run::SaveTuning>()[*i].warn_below;
    }
    check_space();
    out["made"] = kept.made;
    out["snapshot"] = text_of(kept.snapshot);
    out["replayed"] = static_cast<int64_t>(kept.replayed);
    out["frontier"] = frontier;
    out["was_at"] = kept.was_at;
    return out;
}

void KdWorld::check_space() {
    struct statvfs s {};
    free_mb_ = statvfs(folder_.c_str(), &s) == 0
                   ? static_cast<int64_t>(s.f_bavail) * static_cast<int64_t>(s.f_frsize) / (int64_t{1} << 20U)
                   : -1;
}

void KdWorld::save() {
    if (!runner_ || !keeper_) {
        return;
    }
    since_save_ = 0.0;
    check_space();
    runner_->call([this] { snapshot(); });
}

void KdWorld::snapshot() {
    const auto started = std::chrono::steady_clock::now();
    keeper_->snapshot(crowd_->world());
    save_ms_.store(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count(),
                   std::memory_order_relaxed);
}

void KdWorld::save_now() {
    if (!runner_ || !keeper_) {
        return;
    }
    since_save_ = 0.0;
    // the world stops after the batch it is in; the mark and the snapshot meet it between batches
    runner_->set_goal(runner_->frontier());
    runner_->call_and_wait([this] {
        const time::Seconds frontier = crowd_->world().frontier();
        keeper_->pause_mark(std::max(frontier, catch_up_to_));
        snapshot();
        keeper_->flush();
    });
}

void KdWorld::call_home(int64_t camp) {
    if (!runner_ || !stepper_ || camp < 0 || static_cast<std::size_t>(camp) >= stepper_->camp_ids().size()) {
        return;
    }
    const ecs::Id id = stepper_->camp_ids()[static_cast<std::size_t>(camp)];
    runner_->call([this, id] { called_home(id); });
}

void KdWorld::called_home(ecs::Id camp) {
    world::World& w = crowd_->world();
    const world::Command c =
        w.command(w.frontier(), static_cast<std::uint32_t>(demo::Commanded::call_home), camp.value, 0);
    if (keeper_) {
        keeper_->command(c);
    }
}

void KdWorld::call_home_at(int64_t camp, int64_t second) {
    if (!marked_ || !stepper_ || camp < 0 || static_cast<std::size_t>(camp) >= stepper_->camp_ids().size()) {
        return;
    }
    const ecs::Id id = stepper_->camp_ids()[static_cast<std::size_t>(camp)];
    marked_->at(second, [this, id] { called_home(id); });
}

void KdWorld::mark(int64_t second) {
    if (marked_) {
        marked_->mark(second);
    }
}

godot::Dictionary KdWorld::marks() const {
    godot::Dictionary out;
    if (marked_) {
        for (const auto& [second, digest] : marked_->digests()) {
            out[static_cast<int64_t>(second)] = num::to_hex(digest).c_str();
        }
    }
    return out;
}

void KdWorld::reach(int64_t moment) {
    if (runner_) {
        runner_->set_goal(moment);
    }
}

int64_t KdWorld::nearest_camp(int64_t east, int64_t north, int64_t within) const {
    if (!stepper_) {
        return -1;
    }
    const num::Torus& torus = world::World::kTorus;
    const num::Point at = torus.wrap(east, north);
    int64_t best = -1;
    std::int64_t nearest = within * within;
    const std::vector<num::Point>& camps = stepper_->camps();
    for (std::size_t i = 0; i < camps.size(); ++i) {
        const std::int64_t d = torus.squared_distance(at, camps[i]);
        if (d <= nearest) {
            nearest = d;
            best = static_cast<int64_t>(i);
        }
    }
    return best;
}

godot::String KdWorld::digest() const {
    if (!crowd_ || (runner_ && runner_->frontier() != crowd_->world().frontier())) {
        return {};
    }
    return num::to_hex(crowd_->world().digests().whole).c_str();
}

godot::PackedInt64Array KdWorld::camp_at(int64_t camp) const {
    godot::PackedInt64Array out;
    if (stepper_ && camp >= 0 && static_cast<std::size_t>(camp) < stepper_->camps().size()) {
        const num::Point p = stepper_->camps()[static_cast<std::size_t>(camp)];
        out.push_back(p.x);
        out.push_back(p.y);
    }
    return out;
}

bool KdWorld::catching_up() const {
    return catch_up_to_ >= 0;
}

godot::String KdWorld::moment_text(int64_t second) {
    const time::Date d = time::date_of(second);
    return (time::date_text(d) + ", " + time::time_of_day_text(d)).c_str();
}

int64_t KdWorld::crowd_seed() {
    return static_cast<int64_t>(demo::kCrowdSeed);
}

int64_t KdWorld::morning() {
    return demo::kMorning;
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
    rules.margin = ratio(h.margin);
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
    time_requests_.pause(true);
}

void KdWorld::play() {
    pace_.play();
    time_requests_.pause(false);
}

bool KdWorld::is_paused() const {
    return pace_.paused();
}

void KdWorld::frame() {
    const TraceSection section("kd frame");
    if (!runner_) {
        return;
    }
    if (keeper_ && keeper_->failed()) {
        // the phone could not save the world: it stops where the folder keeps it whole, and stays stopped (PLT-07)
        pace_.pause();
    }
    // each frame's own real time, from the steady clock: Godot's delta is smoothed and can hide a stall
    const auto now = std::chrono::steady_clock::now();
    const double real = framed_ ? std::chrono::duration<double>(now - last_frame_).count() : 0.0;
    last_frame_ = now;
    framed_ = true;
    if (catch_up_to_ >= 0) {
        // a reopened world catches up to where it was before the screen moves on from there
        if (runner_->frontier() < catch_up_to_) {
            return;
        }
        const double speed = pace_.speed();
        const bool paused = pace_.paused();
        pace_ = Pace(static_cast<double>(catch_up_to_));
        pace_.set_speed(speed);
        if (paused) {
            pace_.pause();
        }
        catch_up_to_ = -1;
    }
    since_save_ += real;
    if (keeper_ && !pace_.paused()) {
        // the real time the world runs under this version, which keeps the previous version's save an hour (PLT-09)
        played_ += real;
        if (played_ >= 1.0) {
            const auto whole = static_cast<std::int64_t>(played_);
            keeper_->played(whole);
            played_ -= static_cast<double>(whole);
        }
    }
    if (keeper_ && since_save_ >= save_every_) {
        save();
    }
    if (stepper_) {
        // the crowd is asked for no more than the phone can do, less what the heat holds back
        const double can = stepper_->capacity();
        if (can > 0.0) {
            pace_.set_limit(std::max(1.0, can * heat_.share() * kUse));
        }
    }
    if (time_requests_enabled_) {
        if (pace_.limit() < 1e299) time_requests_.capacity(pace_.limit());
        time_requests_.pause(pace_.paused());
        const auto request = time_requests_.resolve();
        if (request.rate >= 1.0) pace_.set_speed(request.rate);
    }
    runner_->set_goal(pace_.frame(real, runner_->frontier()));
    if (stepper_) {
        display_.acquire(*stepper_);
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
    if (keeper_) {
        out["saves"] = static_cast<int64_t>(keeper_->snapshots());
        out["save_ms"] = save_ms_.load(std::memory_order_relaxed);
        out["saved_at"] = keeper_->last_snapshot();
        out["save_bytes"] = static_cast<int64_t>(keeper_->last_snapshot_bytes());
        out["mismatches"] = static_cast<int64_t>(keeper_->mismatches());
        out["save_failed"] = keeper_->failed();
        out["played"] = keeper_->played();
        out["free_mb"] = free_mb_;
        out["warn_below_mb"] = warn_below_mb_;
    }
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

void KdWorld::set_heat_light(double threshold) {
    heat_.set_light(threshold);
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
    if (stepper_) {
        display_.acquire(*stepper_);
    }
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

namespace kd::view {

godot::Array KdWorld::people() const {
    godot::Array out;
    const auto& snapshot = display_.snapshot();

    for (std::size_t i = 0; i < snapshot.walkers.size(); ++i) {
        const auto& walker = snapshot.walkers[i];
        if (!walker.person) continue;
        const auto& p = *walker.person;
        godot::Dictionary row;
        row["id"] = static_cast<int64_t>(walker.id);
        row["name"] = godot::String::utf8(world::kPersonNames[p.name_index].data());
        row["age_at_start"] = static_cast<int64_t>(p.age_years);
        row["appearance"] = static_cast<int64_t>(p.appearance);
        row["camp"] = static_cast<int64_t>(walker.camp);
        const auto at =
            snapshot.way_at(i, screen_time()).at(world::World::kTorus, static_cast<time::Seconds>(screen_time()));
        row["east_cm"] = at.x;
        row["north_cm"] = at.y;
        row["activity"] = "Idle";
        out.push_back(row);
    }
    return out;
}

godot::Dictionary KdWorld::camp_alpha() const {
    godot::Dictionary out;
    const auto& supplies = display_.snapshot().supplies;
    if (supplies.empty()) return out;
    const auto& camp = supplies.front();
    out["half_width_cm"] = camp.half_width_cm;
    out["half_height_cm"] = camp.half_height_cm;
    out["water_ml"] = camp.water_ml;
    out["food_mg"] = camp.food_mg;
    out["stone_mg"] = camp.stone_mg;
    out["wood_mg"] = camp.wood_mg;
    for (const auto& [name, point] :
         std::array<std::pair<const char*, num::Point>, 5>{{{"water_at", camp.water_at},
                                                            {"food_at", camp.food_at},
                                                            {"stone_at", camp.stone_at},
                                                            {"wood_at", camp.wood_at},
                                                            {"shelter_at", camp.shelter_at}}}) {
        godot::PackedInt64Array at;
        at.push_back(point.x);
        at.push_back(point.y);
        out[name] = at;
    }
    return out;
}

}  // namespace kd::view
