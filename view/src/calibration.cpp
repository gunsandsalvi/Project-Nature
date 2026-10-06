#include "calibration.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <array>
#include <string>
#include <utility>

#include "kd/data/catalogue.hpp"

namespace kd::view {

namespace {

godot::String text_of(const std::string& s) {
    return godot::String::utf8(s.c_str());
}

// A reading's fields, by the names the page gives them.
constexpr std::pair<const char*, std::int64_t kd::look::CalibrationReading::*> kFields[] = {
    {"gpu_us", &kd::look::CalibrationReading::gpu_us},   {"cpu_us", &kd::look::CalibrationReading::cpu_us},
    {"on_time", &kd::look::CalibrationReading::on_time}, {"power_mw", &kd::look::CalibrationReading::power_mw},
    {"heat", &kd::look::CalibrationReading::heat},
};

kd::look::CalibrationReading reading_of(const godot::Dictionary& d) {
    kd::look::CalibrationReading r;
    for (const auto& [name, field] : kFields) {
        r.*field = static_cast<std::int64_t>(d.get(name, -1));
    }
    return r;
}

godot::Dictionary dictionary_of(const kd::look::CalibrationReading& r) {
    godot::Dictionary d;
    for (const auto& [name, field] : kFields) {
        d[name] = r.*field;
    }
    return d;
}

}  // namespace

godot::PackedStringArray KdCalibration::read(const godot::PackedStringArray& paths) {
    std::vector<data::SourceFile> files;
    godot::PackedStringArray problems;
    for (const godot::String& path : paths) {
        const godot::PackedByteArray b = godot::FileAccess::get_file_as_bytes("res://data/" + path);
        if (b.is_empty()) {
            problems.append(path + godot::String(": the file cannot be read"));
            continue;
        }
        files.push_back({path.utf8().get_data(), std::string(b.ptr(), b.ptr() + b.size())});
    }
    kd::look::CalibrationSet set = kd::look::read_calibrations(files);
    for (const std::string& p : set.problems) {
        problems.append(text_of(p));
    }
    scenes_ = problems.is_empty() ? std::move(set.scenes) : std::vector<kd::look::CalibrationScene>();
    return problems;
}

godot::Array KdCalibration::scenes() const {
    godot::Array out;
    for (const kd::look::CalibrationScene& s : scenes_) {
        godot::Dictionary scene;
        scene["name"] = text_of(s.name);
        scene["about"] = text_of(s.about);
        scene["draws"] = text_of(s.draws);
        scene["path"] = text_of(s.path);
        scene["measure"] = text_of(s.measure);
        godot::Array variants;
        for (const kd::look::CalibrationVariant& v : s.variants) {
            godot::Dictionary variant;
            variant["name"] = text_of(v.name);
            variant["msaa"] = v.msaa;
            variant["scale"] = v.scale;
            variant["interface"] = v.interface;
            variant["shadows"] = v.shadows;
            variant["triangles"] = v.triangles;
            variant["copies"] = v.copies;
            variant["passes"] = v.passes;
            variants.push_back(variant);
        }
        scene["variants"] = variants;
        out.push_back(scene);
    }
    return out;
}

std::vector<std::vector<kd::look::CalibrationReading>> KdCalibration::readings_of(const godot::Array& readings) const {
    std::vector<std::vector<kd::look::CalibrationReading>> out;
    for (std::size_t s = 0; s < scenes_.size(); ++s) {
        const auto i = static_cast<int64_t>(s);
        const godot::Array each = i < readings.size() ? godot::Array(readings[i]) : godot::Array();
        std::vector<kd::look::CalibrationReading> scene;
        for (std::size_t v = 0; v < scenes_[s].variants.size(); ++v) {
            const auto j = static_cast<int64_t>(v);
            scene.push_back(j < each.size() ? reading_of(each[j]) : kd::look::CalibrationReading());
        }
        out.push_back(scene);
    }
    return out;
}

godot::String KdCalibration::code(int64_t build, const godot::Array& readings) const {
    return text_of(kd::look::calibration_code(build, scenes_, readings_of(readings)));
}

godot::PackedStringArray KdCalibration::verdicts(const godot::Array& readings) const {
    godot::PackedStringArray out;
    const std::vector<kd::look::CalibrationVerdict> verdicts =
        kd::look::calibration_verdicts(scenes_, readings_of(readings));
    for (std::size_t i = 0; i < scenes_.size(); ++i) {
        out.append(text_of(kd::look::verdict_words(scenes_[i], verdicts[i])));
    }
    return out;
}

godot::String KdCalibration::reading_words(int64_t scene, int64_t variant, const godot::Dictionary& reading) const {
    if (scene < 0 || scene >= static_cast<int64_t>(scenes_.size()) || variant < 0 ||
        variant >= static_cast<int64_t>(scenes_[static_cast<std::size_t>(scene)].variants.size())) {
        return {};
    }
    const kd::look::CalibrationScene& s = scenes_[static_cast<std::size_t>(scene)];
    return text_of(kd::look::reading_words(s.variants[static_cast<std::size_t>(variant)], reading_of(reading)));
}

godot::Dictionary KdCalibration::read_code(const godot::String& code) const {
    const kd::look::CalibrationCodeRead read = kd::look::read_calibration_code(code.utf8().get_data(), scenes_);
    godot::Dictionary out;
    out["why"] = text_of(read.why);
    out["build"] = read.build;
    godot::Array readings;
    for (const std::vector<kd::look::CalibrationReading>& scene : read.readings) {
        godot::Array each;
        for (const kd::look::CalibrationReading& r : scene) {
            each.push_back(dictionary_of(r));
        }
        readings.push_back(each);
    }
    out["readings"] = readings;
    return out;
}

void KdCalibration::_bind_methods() {
    using godot::ClassDB;
    using godot::D_METHOD;
    ClassDB::bind_method(D_METHOD("read", "paths"), &KdCalibration::read);
    ClassDB::bind_method(D_METHOD("scenes"), &KdCalibration::scenes);
    ClassDB::bind_method(D_METHOD("code", "build", "readings"), &KdCalibration::code);
    ClassDB::bind_method(D_METHOD("verdicts", "readings"), &KdCalibration::verdicts);
    ClassDB::bind_method(D_METHOD("reading_words", "scene", "variant", "reading"), &KdCalibration::reading_words);
    ClassDB::bind_method(D_METHOD("read_code", "code"), &KdCalibration::read_code);
}

}  // namespace kd::view
