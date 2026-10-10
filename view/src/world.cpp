#include "world.hpp"
#include <limits>
#include "kd/demo/crafting.hpp"
#include "kd/demo/discovery.hpp"
#include "kd/demo/fire.hpp"
#include "kd/demo/idea_dreams.hpp"
#include "kd/demo/learning.hpp"

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
    ClassDB::bind_method(D_METHOD("start_crowd", "seed", "camps"), &KdWorld::start_crowd);
    ClassDB::bind_method(D_METHOD("open_crowd", "folder", "seed", "camps", "build"), &KdWorld::open_crowd);
    ClassDB::bind_method(D_METHOD("open_camp", "folder", "seed", "build"), &KdWorld::open_camp);
    ClassDB::bind_method(D_METHOD("items", "person"), &KdWorld::items, DEFVAL(0));
    ClassDB::bind_method(D_METHOD("knowledge", "person"), &KdWorld::knowledge);
    ClassDB::bind_method(D_METHOD("craft_history"), &KdWorld::craft_history);
    ClassDB::bind_method(D_METHOD("people"), &KdWorld::people);
    ClassDB::bind_method(D_METHOD("camp_alpha"), &KdWorld::camp_alpha);
    ClassDB::bind_method(D_METHOD("save"), &KdWorld::save);
    ClassDB::bind_method(D_METHOD("save_now"), &KdWorld::save_now);
    ClassDB::bind_method(D_METHOD("camp_at", "camp"), &KdWorld::camp_at);
    ClassDB::bind_method(D_METHOD("catching_up"), &KdWorld::catching_up);
    ClassDB::bind_method(D_METHOD("digest"), &KdWorld::digest);
    ClassDB::bind_method(D_METHOD("prepare_dream"), &KdWorld::prepare_dream);
    ClassDB::bind_method(D_METHOD("dream_subjects", "person"), &KdWorld::dream_subjects);
    ClassDB::bind_method(D_METHOD("idea_memories", "person"), &KdWorld::idea_memories);
    ClassDB::bind_method(D_METHOD("send_idea_dream", "person", "memory"), &KdWorld::send_idea_dream);
    ClassDB::bind_method(D_METHOD("send_place_dream", "person", "subject"), &KdWorld::send_place_dream);
    ClassDB::bind_method(D_METHOD("dream_records"), &KdWorld::dream_records);
    ClassDB::bind_static_method("KdWorld", D_METHOD("moment_text", "second"), &KdWorld::moment_text);
    ClassDB::bind_static_method("KdWorld", D_METHOD("crowd_seed"), &KdWorld::crowd_seed);
    ClassDB::bind_static_method("KdWorld", D_METHOD("morning"), &KdWorld::morning);
    ClassDB::bind_static_method("KdWorld", D_METHOD("save_format"), &KdWorld::save_format);
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
    demo::Kept kept = demo::keep_crowd(*keeper_, *catalogue_, static_cast<std::uint64_t>(seed), camps, camp_alpha);
    godot::PackedStringArray damaged;
    for (const std::string& d : kept.damaged) {
        damaged.append(text_of(d));
    }
    out["damaged"] = damaged;
    const std::array<const char*, 3> updates{"none", "older", "big"};
    out["update"] = updates.at(static_cast<std::size_t>(kept.update));
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

godot::Dictionary KdWorld::prepare_dream() {
    godot::Dictionary out;
    if (!runner_ || !crowd_ || !crowd_->living() || catching_up()) {
        out["problem"] = "Wait for the camp to finish opening";
        return out;
    }
    const double before = screen_time();
    pause();
    runner_->set_goal(runner_->frontier());
    time::Seconds edge = 0;
    runner_->call_and_wait([this, &edge] {
        edge = crowd_->world().frontier();
        runner_->set_goal(edge);
        stepper_->refresh();
    });
    pace_.settle(edge);
    stepper_->set_screen(static_cast<double>(edge));
    display_.acquire(*stepper_);
    out["displayed_before"] = before;
    out["at"] = edge;
    return out;
}
godot::Array KdWorld::dream_subjects(int64_t person) {
    godot::Array out;
    if (!runner_ || !crowd_ || !crowd_->living() || !is_paused()) return out;
    runner_->call_and_wait([this, person, &out] {
        const auto& w = crowd_->world();
        const ecs::Id id{static_cast<std::uint64_t>(person)};
        const auto h = w.beings().find(id);
        if (!h || !w.beings().raw().all_of<world::Life>(*h)) return;
        const auto& life = w.beings().raw().get<world::Life>(*h);
        const auto live = crowd_->living()->sample(life, w.beings().raw().get<world::Activity>(*h), w.frontier());
        const auto need = demo::Living::needs(live);
        std::vector<std::size_t> subjects;
        for (std::size_t i = 0; i < 3; ++i)
            if (life.source[i] != 0) subjects.push_back(i);
        std::sort(subjects.begin(), subjects.end(), [&](std::size_t a, std::size_t b) {
            if (need[a] != need[b]) return need[a] < need[b];
            if (life.seen[a] != life.seen[b]) return life.seen[a] > life.seen[b];
            return a < b;
        });
        constexpr std::array<const char*, 3> names{"Food plants", "Water", "Shelter"};
        for (const auto i : subjects) {
            godot::Dictionary row;
            row["subject"] = static_cast<int64_t>(i);
            row["name"] = names[i];
            row["seen_at"] = life.seen[i];
            row["problem"] = demo::Living::dream_problem(w, id, static_cast<std::int64_t>(i)).c_str();
            out.push_back(row);
        }
    });
    return out;
}
godot::Array KdWorld::idea_memories(int64_t person) {
    godot::Array out;
    if (!runner_ || !crowd_ || !crowd_->living() || !is_paused()) return out;
    runner_->call_and_wait([this, person, &out] {
        const auto& w = crowd_->world();
        const ecs::Id id{static_cast<std::uint64_t>(person)};
        const auto h = w.beings().find(id);
        if (!h) return;
        const auto* know = w.beings().raw().try_get<world::Knowledge>(*h);
        if (!know) return;
        for (auto it = know->memories.rbegin(); it != know->memories.rend(); ++it) {
            if (it->inputs.empty() || !(know->performed & (1U << it->action))) continue;
            godot::Dictionary row;
            row["memory"] = static_cast<int64_t>(it->id);
            row["at"] = it->at;
            row["action"] = it->action;
            row["name"] = it->action == 11  ? "Twirled dry wood"
                          : it->action == 6 ? "Rubbed materials"
                                            : "Handled familiar materials";
            row["problem"] = demo::IdeaDreams::problem(w, id, it->id).c_str();
            const auto fit = demo::IdeaDreams::fit(w, *h, it->id);
            constexpr std::array<const char*, 18> benefit{
                "resistance", "a sharp edge", "strength", "breaking", "flexibility", "weight",
                "burning",    "fuel",         "food",     "water",    "harm",        "care",
                "warmth",     "fibres",       "sticking", "shaping",  "keeping dry", "colour"};
            row["benefit"] = fit ? benefit[fit->desired_property] : "an experienced benefit";
            out.push_back(row);
        }
    });
    return out;
}
godot::Dictionary KdWorld::send_place_dream(int64_t person, int64_t subject) {
    return send_dream(person, subject, false);
}
godot::Dictionary KdWorld::send_idea_dream(int64_t person, int64_t memory) {
    return send_dream(person, memory, true);
}
godot::Dictionary KdWorld::send_dream(int64_t person, int64_t subject, bool idea) {
    godot::Dictionary out;
    if (!runner_ || !crowd_ || !crowd_->living() || !is_paused() ||
        screen_time() != static_cast<double>(runner_->frontier())) {
        out["problem"] = "Pause at the camp's current moment before choosing";
        return out;
    }
    time::Seconds asked = static_cast<time::Seconds>(screen_time());
    std::string problem;
    std::uint64_t number = 0;
    runner_->call_and_wait([this, person, subject, idea, asked, &problem, &number] {
        auto& w = crowd_->world();
        const ecs::Id id{static_cast<std::uint64_t>(person)};
        if (w.frontier() != asked) {
            problem = "The camp moved; choose again";
            return;
        }
        problem = idea ? demo::IdeaDreams::problem(w, id, static_cast<std::uint64_t>(subject))
                       : demo::Living::dream_problem(w, id, subject);
        if (!problem.empty()) return;
        const auto cmd = w.command(asked, idea ? demo::Living::kIdeaDream : demo::Living::kPlaceDream, id.value,
                                   static_cast<std::uint64_t>(subject));
        if (keeper_) {
            keeper_->command(cmd);
            if (keeper_->failed()) {
                problem = "Camp could not save; the dream has not acted";
                return;
            }
        }
        number = cmd.number;
    });
    if (!problem.empty()) {
        out["problem"] = problem.c_str();
        return out;
    }
    runner_->set_goal(asked + 1);
    runner_->wait_for(asked + 1);
    pace_.settle(asked + 1);
    stepper_->set_screen(static_cast<double>(asked + 1));
    display_.acquire(*stepper_);
    out["number"] = static_cast<int64_t>(number);
    out["requested"] = asked;
    out["received"] = asked;
    return out;
}
godot::Array KdWorld::dream_records() {
    godot::Array out;
    if (!runner_ || !crowd_ || !crowd_->living()) return out;
    runner_->call_and_wait([this, &out] {
        const auto& w = crowd_->world();
        constexpr std::array<const char*, 3> names{"Food plants", "Water", "Shelter"};
        for (const auto camp : stepper_->camp_ids()) {
            const auto* ledger = w.beings().raw().try_get<world::Dreams>(w.beings().handle(camp));
            if (!ledger) continue;
            for (const auto& a : ledger->acts) {
                godot::Dictionary row;
                row["number"] = static_cast<int64_t>(a.number);
                row["person"] = static_cast<int64_t>(a.person);
                row["name"] = "Someone who is gone";
                const auto h = w.beings().find(ecs::Id{a.person});
                if (h && w.beings().raw().all_of<world::Person>(*h)) {
                    const auto& p = w.beings().raw().get<world::Person>(*h);
                    row["name"] = godot::String::utf8(world::kPersonNames[p.name_index].data());
                }
                row["subject"] = a.subject;
                row["kind"] = a.kind;
                row["memory"] = static_cast<int64_t>(a.memory);
                row["place"] =
                    a.kind == 1 ? "An idea from remembered work" : names[static_cast<std::size_t>(a.subject)];
                row["first_attempt_at"] = a.first_attempt_at;
                row["result_at"] = -1;
                row["result"] = "No matching later result recorded";
                if (a.kind == 1 && a.first_attempt_at >= 0) {
                    const auto& history = w.beings().raw().get<world::CraftHistory>(w.beings().handle(camp));
                    for (const auto& result : history.events) {
                        if (result.actor.value != a.person || result.at < a.first_attempt_at ||
                            result.recipe != a.recipe)
                            continue;
                        row["result_at"] = result.at;
                        row["result"] = result.kind == 5  ? godot::String("The attempt failed")
                                        : !result.noticed ? godot::String("A result was not noticed")
                                                          : godot::String("Noticed a result: ") +
                                                                godot::String::utf8(result.word.c_str());
                        break;
                    }
                }
                row["requested"] = a.requested;
                row["received"] = a.received;
                row["executed"] = a.executed;
                row["until"] = a.until;
                row["status"] = a.status;
                row["reason"] = a.reason;
                row["decision_at"] = a.decision_at;
                row["choice"] = a.choice;
                row["pull"] = a.pull;
                row["visited_at"] = a.visited_at;
                row["east_cm"] = a.place.x;
                row["north_cm"] = a.place.y;
                out.push_back(row);
            }
        }
    });
    return out;
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
}

void KdWorld::play() {
    pace_.play();
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
        // Real time played in this build.
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
        const auto k = snapshot.way_index(i, screen_time());
        const auto& activity = snapshot.ways[k];
        constexpr std::array<const char*, 13> activities{"Watching nearby ground",
                                                         "Walking",
                                                         "Resting",
                                                         "",
                                                         "Gathering berries",
                                                         "Eating berries",
                                                         "Drinking at water",
                                                         "Carrying berries",
                                                         "Working",
                                                         "Watching work",
                                                         "Teaching",
                                                         "Warming",
                                                         "Tending"};
        row["activity"] = activities[activity.what];
        row["action_code"] = activity.what;
        row["walk_cm"] = world::World::kTorus.distance(activity.from, at);
        row["action_start"] = activity.start;
        row["action_end"] = activity.end;
        row["progress_ppm"] = activity.share(static_cast<time::Seconds>(screen_time()));
        if (k < snapshot.lives.size()) {
            const auto& saved_life = snapshot.lives[k];
            if (saved_life && crowd()->living()) {
                const auto& recorded = *saved_life;
                const auto thermal = snapshot.thermal_at(i, screen_time(), crowd()->living()->rules().water_day);
                const auto live = crowd()->living()->sample(
                    recorded, activity, static_cast<time::Seconds>(screen_time()), thermal ? thermal->water_due_ml : 0);
                if (thermal) {
                    row["felt_milli_c"] = thermal->felt_milli_c;
                    row["warmth_need"] = thermal->warmth;
                    row["tending"] = thermal->tending;
                    if (activity.what == static_cast<std::uint8_t>(world::LivingAct::warm))
                        row["activity"] = "Resting by fire";
                    if (activity.what == static_cast<std::uint8_t>(world::LivingAct::tend))
                        row["activity"] = "Tending fire";
                }
                const auto need = demo::Living::needs(live);
                row["food_need"] = need[0];
                row["water_need"] = need[1];
                row["rest_need"] = need[2];
                row["choice"] = recorded.goal;
                row["decision_at"] = recorded.decision_at;
                row["carried_food_mg"] = live.carried_food;
                row["gathering_skill"] = recorded.gathering_skill;
                row["memory_at"] = recorded.memory_at;
                row["memory_kind"] = recorded.memory_kind;
                row["memory_amount"] = recorded.memory_amount;
                godot::PackedInt64Array decision, scores, known, seen, sources, benefit, cost, blocked, unavailable;
                for (std::size_t n = 0; n < 3; ++n) {
                    decision.push_back(recorded.decision_needs[n]);
                    benefit.push_back(recorded.benefit[n]);
                    cost.push_back(recorded.cost_seconds[n]);
                    known.push_back(recorded.known_amount[n]);
                    seen.push_back(recorded.seen[n]);
                    sources.push_back(recorded.source[n]);
                    blocked.push_back(recorded.blocked_until[n]);
                    unavailable.push_back(recorded.unavailable[n]);
                }
                for (const auto score : recorded.scores) scores.push_back(score);
                row["decision_needs"] = decision;
                row["scores"] = scores;
                row["known_amounts"] = known;
                row["seen_at"] = seen;
                row["sources"] = sources;
                row["blocked_until"] = blocked;
                row["unavailable"] = unavailable;
                row["benefits"] = benefit;
                row["cost_seconds"] = cost;
            }
        }
        if (k < snapshot.dreams.size()) {
            const auto& thought = snapshot.dreams[k];
            if (thought) {
                row["dream_kind"] = thought->kind;
                row["dream_action"] = thought->action;
                row["dream_at"] = thought->at;
                row["dream_until"] = thought->until;
                row["dream_subject"] = thought->subject;
                row["dream_pull"] = thought->decision_pull;
                row["dream_decision_subject"] = thought->decision_subject;
                row["dream_visit_at"] = thought->visit_at;
            }
        }
        if (k < snapshot.works.size() && snapshot.works[k]) {
            const auto& work = *snapshot.works[k];
            row["work_state"] = work.state;
            row["work_action"] = work.action;
            row["work_route"] = work.route;
            const auto* personal = k < snapshot.knowledge.size() ? snapshot.knowledge[k].get() : nullptr;
            row["work_known"] = work.intended && personal && demo::Learning::knows(*personal, work.recipe);
            row["work_taught"] = work.lesson != 0;
            row["work_recipe"] =
                work.intended ? text_of(catalogue_->kind<data::Blueprint>().name(work.recipe)) : godot::String();
            row["work_start"] = work.start;
            row["work_end"] = work.end;
            row["work_next_try"] = work.next_try;
            row["work_progress"] = work.retained_progress;
            row["work_tries"] = static_cast<int64_t>(work.completed_tries);
            godot::PackedInt64Array inputs;
            for (const auto& r : work.inputs) inputs.push_back(static_cast<int64_t>(r.item.value));
            row["work_inputs"] = inputs;
            if (work.state != 0)
                row["activity"] = work.state == 1   ? "Collecting work inputs"
                                  : work.state == 4 ? "Work paused for bodily needs"
                                  : work.intended   ? godot::String("Making ") + godot::String(row["work_recipe"])
                                                    : "Trying familiar materials";
            if (k < snapshot.lives.size() && snapshot.lives[k] && snapshot.lives[k]->meal_item.value != 0)
                row["activity"] = activity.what == 5 ? "Eating finite food" : "Going to reserved food";
        }
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
    std::int64_t reserved = 0;
    const auto& snapshot = display_.snapshot();
    for (std::size_t i = 0; i < snapshot.walkers.size(); ++i) {
        if (snapshot.walkers[i].camp != 0) continue;
        const auto k = snapshot.first[i + 1] - 1;
        if (k < snapshot.lives.size()) {
            const auto& saved_life = snapshot.lives[k];
            if (saved_life) reserved += saved_life->allocated_water;
        }
    }
    out["reserved_water_ml"] = reserved;
    out["food_mg"] = camp.food_mg;
    out["stone_mg"] = camp.stone_mg;
    out["wood_mg"] = camp.wood_mg;
    if (snapshot.item_first.size() > 1) {
        std::int64_t stone = 0, wood = 0, food = 0;
        for (std::size_t i = 0; i + 1 < snapshot.item_first.size(); ++i) {
            const auto* saved = snapshot.item_at(i, screen_time());
            if (!saved || saved->item.mass == 0) continue;
            const auto physical = demo::Crafting::physical(*catalogue_, saved->item);
            if (physical.material_class == "stone") stone += saved->item.mass;
            if (physical.material_class == "wood") wood += saved->item.mass;
            if (physical.values[8] > 0) food += saved->item.mass;
        }
        out["stone_mg"] = stone;
        out["wood_mg"] = wood;
        out["finite_food_mg"] = food;
        out["discovery"] = true;
    }
    out["settled_frontier"] = display_.snapshot().frontier;
    const auto& habitats = display_.snapshot().habitats;
    if (!habitats.empty()) {
        const auto& env = habitats.front();
        out["rock_west"] = env.rock_west;
        out["rock_east"] = env.rock_east;
        out["rock_south"] = env.rock_south;
        out["rock_north"] = env.rock_north;
        out["upstream_ml"] = env.upstream_ml;
        out["root_water_ml"] = env.root_water_ml;
        out["crop_budget_mg"] = env.crop_budget_mg;
        out["food_grown_mg"] = env.food_grown;
        out["water_added_ml"] = env.water_added;
        out["food_taken_mg"] = env.food_taken;
        out["water_taken_ml"] = env.water_taken;
        out["water_spilled_ml"] = env.water_spilled;
    }
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

namespace kd::view {
namespace {
const world::Knowledge* recorded_knowledge(const Snapshot& s, std::uint64_t person, double at) {
    for (std::size_t i = 0; i < s.walkers.size(); ++i) {
        if (s.walkers[i].id != person) continue;
        const auto k = s.way_index(i, at);
        if (k < s.knowledge.size() && s.knowledge[k]) return s.knowledge[k].get();
    }
    return nullptr;
}
godot::String craft_label(std::string_view name) {
    const auto colon = name.find(':');
    std::string label(name.substr(colon == std::string_view::npos ? 0 : colon + 1));
    std::replace(label.begin(), label.end(), '_', ' ');
    if (!label.empty() && label[0] >= 'a' && label[0] <= 'z') label[0] += 'A' - 'a';
    return godot::String::utf8(label.c_str());
}
godot::Dictionary evidence(const world::Familiar& f) {
    godot::Dictionary row;
    row["mask"] = f.mask;
    row["edible"] = bool(f.edible);
    row["edible_source"] = f.edible_source;
    row["seen_at"] = f.at;
    godot::PackedInt64Array values, certainty, sources, times, people, events;
    for (std::size_t i = 0; i < 18; ++i) {
        values.push_back(f.values[i]);
        certainty.push_back(f.certainty[i]);
        sources.push_back(f.sources[i]);
        times.push_back(f.learned_at[i]);
        people.push_back(static_cast<int64_t>(f.source_people[i].value));
        events.push_back(static_cast<int64_t>(f.source_events[i]));
    }
    row["values"] = values;
    row["certainty"] = certainty;
    row["sources"] = sources;
    row["learned_at"] = times;
    row["source_people"] = people;
    row["source_events"] = events;
    return row;
}
}  // namespace

godot::Array KdWorld::items(int64_t person) const {
    godot::Array out;
    const auto& s = display_.snapshot();
    for (std::size_t i = 0; i + 1 < s.item_first.size(); ++i) {
        const auto* saved = s.item_at(i, screen_time());
        if (!saved || saved->item.mass == 0) continue;
        const auto& item = saved->item;
        godot::Dictionary row;
        row["id"] = static_cast<int64_t>(saved->id.value);
        row["kind"] = text_of(catalogue_->kind<data::ItemKind>().name(item.kind));
        row["name"] = craft_label(catalogue_->kind<data::ItemKind>().name(item.kind));
        row["form"] = text_of(catalogue_->kind<data::ItemKind>()[item.kind].form);
        row["material"] = craft_label(catalogue_->kind<data::ItemKind>().name(item.material));
        row["mass_mg"] = item.mass;
        row["length_mm"] = item.length;
        row["quality"] = item.quality;
        row["wear"] = item.wear;
        row["state"] = item.state;
        row["owner"] = static_cast<int64_t>(item.owner.value);
        row["maker"] = static_cast<int64_t>(item.maker.value);
        row["made_at"] = item.made_at;
        row["east_cm"] = saved->place.at.x;
        row["north_cm"] = saved->place.at.y;
        // A held tool follows the holder's immutable sampled way, never their later live position.
        for (std::size_t walker = 0; walker < s.walkers.size(); ++walker) {
            if (s.walkers[walker].id != item.owner.value) continue;
            const auto at =
                s.way_at(walker, screen_time()).at(world::World::kTorus, static_cast<time::Seconds>(screen_time()));
            row["east_cm"] = at.x;
            row["north_cm"] = at.y;
        }
        const auto observer = person > 0 ? static_cast<std::uint64_t>(person) : item.owner.value;
        row["observer"] = static_cast<int64_t>(observer);
        if (saved->fire) {
            const auto& fire = *saved->fire;
            const auto rate = fire.heat >= 3 ? 5000000 : fire.heat == 2 ? 1000000 : 0;
            const auto elapsed =
                std::max<time::Seconds>(0, static_cast<time::Seconds>(screen_time()) - fire.settled_at);
            const auto fuel =
                std::max<std::int64_t>(0, fire.fuel_mg - (elapsed * rate + fire.burn_remainder) / time::kHour);
            row["fire_heat"] = fire.heat;
            row["fuel_mg"] = fuel;
            row["ash_mg"] = fire.ash_mg + fire.fuel_mg - fuel;
            row["fuel_seconds"] =
                rate ? (fuel * time::kHour - (elapsed * rate + fire.burn_remainder) % time::kHour + rate - 1) / rate
                     : 0;
            row["name"] = fire.heat >= 2 ? "Campfire" : fire.heat == 1 ? "Embers" : "Cold hearth";
        }
        if (saved->timer) {
            const auto& timer = *saved->timer;
            const auto elapsed =
                timer.exposure_heat >= 2 && !timer.completed
                    ? std::max<time::Seconds>(0, static_cast<time::Seconds>(screen_time()) - timer.settled_at)
                    : 0;
            row["heat_exposure_seconds"] = std::min(2 * time::kHour, timer.elapsed + elapsed);
            row["cooking_paused"] = timer.exposure_heat < 2;
            row["cooking_tried"] = timer.tried != 0;
            row["cooking_next"] = timer.next;
        }
        if ((catalogue_->find("item", "base:roots") == item.kind ||
             catalogue_->find("item", "base:meat") == item.kind) &&
            item.state > 0 && item.state < 3)
            row["name"] = godot::String(item.state == 1 ? "Cooked " : "Burnt ") +
                          craft_label(catalogue_->kind<data::ItemKind>().name(item.kind));
        row["facts"] = godot::Dictionary();
        if (const auto* know = recorded_knowledge(s, observer, screen_time())) {
            if (const auto* f = demo::Discovery::familiar(*know, item)) {
                row["facts"] = evidence(*f);
                if (catalogue_->kind<data::ItemKind>()[item.kind].form == "flake" && (f->mask & (1U << 1U)) &&
                    f->values[1] >= 3)
                    row["name"] = "Sharp flake";
            }
        }
        out.push_back(row);
    }
    return out;
}
godot::Dictionary KdWorld::knowledge(int64_t person) const {
    godot::Dictionary out;
    const auto* know = recorded_knowledge(display_.snapshot(), static_cast<std::uint64_t>(person), screen_time());
    if (!know) return out;
    out["curiosity"] = know->curiosity;
    out["kindness"] = know->kindness;
    out["curiosity_need"] = know->curiosity_need;
    out["performed"] = know->performed;
    godot::Array skills, observations, familiar, hunches, reasons;
    for (const auto& s : know->skills) {
        godot::Dictionary row;
        if (!s.known) {
            row["credits"] = static_cast<double>(s.observation_quarters) / 4.0 +
                             static_cast<double>(s.observation_remainder) / 4000000.0;
            observations.push_back(row);
            continue;
        }
        row["recipe"] = text_of(catalogue_->kind<data::Blueprint>().name(s.recipe));
        row["name"] = craft_label(catalogue_->kind<data::Blueprint>().name(s.recipe));
        row["level"] = s.practice.level;
        row["last_use"] = s.practice.last_use;
        row["source"] = static_cast<int64_t>(s.source.value);
        row["source_event"] = static_cast<int64_t>(s.source_event);
        row["route"] = s.route;
        skills.push_back(row);
    }
    for (const auto& f : know->familiar) {
        auto row = evidence(f);
        row["kind"] = text_of(catalogue_->kind<data::ItemKind>().name(f.kind));
        row["name"] = craft_label(catalogue_->kind<data::ItemKind>().name(f.kind));
        familiar.push_back(row);
    }
    for (const auto& h : know->hunches) {
        godot::Dictionary row;
        row["action"] = h.action;
        row["source_memory"] = static_cast<int64_t>(h.source_memory);
        row["source"] = static_cast<int64_t>(h.source.value);
        row["origin"] = h.origin;
        row["failures"] = h.failures;
        row["last_use"] = h.last_use;
        godot::Array inputs;
        for (const auto& f : h.inputs) inputs.push_back(craft_label(catalogue_->kind<data::ItemKind>().name(f.kind)));
        row["inputs"] = inputs;
        hunches.push_back(row);
    }
    for (const auto& r : know->reasons) {
        godot::Dictionary row;
        row["known"] = r.intended && demo::Learning::knows(*know, r.recipe);
        row["action"] = r.action;
        row["name"] = bool(row["known"]) ? craft_label(catalogue_->kind<data::Blueprint>().name(r.recipe))
                      : r.intended       ? godot::String("Shared practice")
                                         : godot::String("Try familiar materials");
        row["score"] = r.score;
        row["benefit"] = r.benefit;
        row["seconds"] = r.seconds;
        reasons.push_back(row);
    }
    out["skills"] = skills;
    out["observations"] = observations;
    out["session"] = static_cast<int64_t>(know->session);
    out["familiar"] = familiar;
    out["hunches"] = hunches;
    out["reasons"] = reasons;
    return out;
}
godot::Array KdWorld::craft_history() const {
    godot::Array out;
    for (const auto& history : display_.snapshot().craft_history) {
        for (const auto& e : history.events) {
            if (static_cast<double>(e.at) > screen_time()) continue;
            godot::Dictionary row;
            row["id"] = static_cast<int64_t>(e.id);
            row["at"] = e.at;
            row["actor"] = static_cast<int64_t>(e.actor.value);
            row["source"] = static_cast<int64_t>(e.source.value);
            row["result"] = static_cast<int64_t>(e.result.value);
            row["recipe"] = text_of(catalogue_->kind<data::Blueprint>().name(e.recipe));
            row["name"] = craft_label(catalogue_->kind<data::Blueprint>().name(e.recipe));
            row["kind"] = e.kind;
            row["noticed"] = bool(e.noticed);
            row["route"] = e.route;
            row["word"] = text_of(e.word);
            godot::PackedInt64Array inputs;
            for (const auto& r : e.inputs) inputs.push_back(static_cast<int64_t>(r.id.value));
            row["inputs"] = inputs;
            row["east_cm"] = e.place.x;
            row["north_cm"] = e.place.y;
            out.push_back(row);
        }
    }
    return out;
}
}  // namespace kd::view
