#include "world.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include "kd/core/check.hpp"
#include "kd/num/convert.hpp"
#include "kd/num/digest.hpp"
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
}

namespace {

godot::String text_of(const std::string& s) {
    return godot::String::utf8(s.c_str());
}

}  // namespace

godot::Dictionary KdWorld::load_catalogue(const godot::PackedStringArray& paths) {
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
    runner_->set_goal(pace_.frame(real, runner_->frontier()));
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

}  // namespace kd::view
