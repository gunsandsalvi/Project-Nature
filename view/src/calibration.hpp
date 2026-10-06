// The calibration's scenes for the Calibrate page (A18.1, α2.2a): it reads the scene files the build lists, gives the
// page each scene's switches, and turns the page's readings into the verdicts and the one code through kd::look's
// calibration, the same the cloud reads the code with. It converts and never decides for the world (WLD-13).
#pragma once

#include <cstdint>
#include <vector>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include "kd/look/calibration.hpp"

namespace kd::view {

/// Implements PLT-04 and RES-09, see A18.1: the Calibrate page's scenes, readings, verdicts and code.
class KdCalibration : public godot::RefCounted {
    GDCLASS(KdCalibration, godot::RefCounted)

public:
    /// Reads the scene files under res://data/ by their paths, such as "scenes/look/c4.toml": what is wrong with
    /// them, in words, empty when nothing is.
    godot::PackedStringArray read(const godot::PackedStringArray& paths);
    /// The scenes read, in the order they run: each a dictionary of name, about, draws, path, measure and variants,
    /// each variant a dictionary of name, msaa, scale (percent), interface, shadows, triangles (thousands), copies and
    /// passes.
    [[nodiscard]] godot::Array scenes() const;
    /// The page's readings as one code with the app's build: an array for each scene of a dictionary for each
    /// variant, with gpu_us, cpu_us, on_time (per thousand), power_mw and heat (hundredths), each -1 or missing where
    /// the phone gave none.
    [[nodiscard]] godot::String code(int64_t build, const godot::Array& readings) const;
    /// Each scene's verdict in words, from the same readings.
    [[nodiscard]] godot::PackedStringArray verdicts(const godot::Array& readings) const;
    /// A variant's readings in words.
    [[nodiscard]] godot::String reading_words(int64_t scene, int64_t variant, const godot::Dictionary& reading) const;
    /// A code read back against the scenes, for the tests: why it cannot be read, or "", the build and the readings
    /// as code() takes them.
    [[nodiscard]] godot::Dictionary read_code(const godot::String& code) const;

protected:
    static void _bind_methods();

private:
    [[nodiscard]] std::vector<std::vector<kd::look::CalibrationReading>> readings_of(
        const godot::Array& readings) const;

    std::vector<kd::look::CalibrationScene> scenes_;
};

}  // namespace kd::view
