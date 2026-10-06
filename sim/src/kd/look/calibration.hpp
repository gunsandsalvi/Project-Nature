// The calibration scenes (A18.1, α2.2a): what only your phone can measure. Each scene is stated in a file before its
// first run (RES-09): what it draws, its camera, its variants' switches, the number that decides, the line from A18.1's
// budget, the estimate, and the decision each band of the number makes. The phone runs the scenes you pick and writes
// all their readings in one code (kd/num/letters.hpp); the cloud reads the code against the same files. Written once,
// here, for both.
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "kd/data/catalogue.hpp"
#include "kd/data/toml.hpp"

namespace kd::look {

/// One variant of a calibration scene: its switches.
struct CalibrationVariant {
    std::string name;
    std::int64_t msaa = 2;       // samples: 0 (off), 2 or 4
    std::int64_t scale = 100;    // the 3D's scale, in percent of the screen's resolution
    bool interface = true;       // the app's interface drawn over the world
    bool shadows = true;         // the sun's shadow pass
    std::int64_t triangles = 0;  // for rocks: thousands of triangles a pass
    std::int64_t copies = 0;     // for copies: draws of copies
    std::int64_t passes = 2;     // for copies: 2, the main pass and the sun's shadow, or 3 with a mirror's pass
    std::string way;             // for leaves, fires and figures: the way they are drawn, as calibration_ways names
    std::int64_t fires = 0;      // for fires: the fires burning in view
    std::int64_t figures = 0;    // for figures: the figures in view
    std::int64_t vertices = 0;   // for reads: thousands of vertices
    std::int64_t reads = 0;      // for reads: the texture pixels each vertex reads
};

/// What a number at most at_most, or any for the last, makes the plan do.
struct CalibrationDecision {
    std::optional<std::int64_t> at_most;
    std::string then;
};

/// Implements PLT-04 and RES-09, see A18.1: a calibration scene as its file states it.
struct CalibrationScene {
    std::string name;  // its file's, without .toml
    std::string about;
    std::string step;                 // the step whose run it belongs to, such as α2.2a; the page offers those first
    std::vector<std::string> checks;  // the items it serves, by ID
    // what it draws: "nothing", "field" (the full material over the screen), "rocks", "copies", "leaves" (plants at
    // the liked density), "fires" (fires and what stands round them at night), "figures" (posed figures) or "reads"
    // (vertices reading texture pixels)
    std::string draws;
    std::string path;  // the camera: "still", "pan", "turn" or "pinch"
    // what decides: "gpu" or "cpu", microseconds a frame; "gpu_per_triangle", picoseconds; "cpu_per_figure",
    // nanoseconds of the main thread a figure drawn by Godot's own skeletons
    std::string measure;
    std::string decides;                // for gpu and cpu: the variant whose number decides
    std::vector<std::string> least_of;  // for gpu and cpu, instead: the variants whose least number decides
    std::string minus;                  // for gpu and cpu: "scene/variant", whose same number is taken off, or none
    std::int64_t line = 0;              // A18.1's allowance, in the measure's unit
    std::int64_t expect_from = 0;       // the estimate, in the measure's unit
    std::int64_t expect_to = 0;
    std::vector<CalibrationVariant> variants;
    std::vector<CalibrationDecision> decisions;
};

/// The ways a scene's drawing may be drawn, by what it draws, none for the rest: leaves as none (the bare ground),
/// plain cut-out cards, cards cut close to their leaves, solid cores with cut-out fringes, or close-cut cards with
/// alpha to coverage; fires' shadows by none, a walk at every pixel, a walk at half resolution, or a map for each
/// fire; figures by Godot's own skeletons or by our bone palettes.
[[nodiscard]] std::vector<std::string_view> calibration_ways(std::string_view draws);

/// A scene read: it, and what is wrong with its file, empty when nothing is.
struct CalibrationRead {
    CalibrationScene scene;
    std::vector<data::Problem> problems;
};

/// Implements PLT-04 and RES-09, see A18.1: a calibration scene from its file's text, its name from its path; refused
/// without the items it serves, a variant, the number that decides, its line, its estimate and a decision for every
/// number.
[[nodiscard]] CalibrationRead read_calibration(std::string_view text, const std::string& path);

/// One variant's readings on the phone, each -1 where the phone gave none.
struct CalibrationReading {
    std::int64_t gpu_us = -1;    // the graphics chip's mean time a frame at the 120 cap, every viewport summed
    std::int64_t cpu_us = -1;    // the main thread's mean time drawing a frame at the 120 cap, every viewport summed
    std::int64_t on_time = -1;   // frames on time at the 60 cap, per thousand
    std::int64_t power_mw = -1;  // the phone's mean power at the 60 cap, in milliwatts
    std::int64_t heat = -1;      // the heat forecast at the 60 cap's end, in hundredths of severe throttling
};

/// Implements PLT-04, see A18.1: what is wrong across a set of scenes, in words: a minus naming a scene or variant
/// not among them, two scenes of one name, or more scenes than a code holds.
[[nodiscard]] std::vector<std::string> check_calibrations(const std::vector<CalibrationScene>& scenes);

/// The most scenes one code holds.
inline constexpr std::size_t kCalibrationScenesMost = 16;

/// What a scene's readings decide.
struct CalibrationVerdict {
    bool ran = false;         // the scene was run
    bool read = false;        // its readings were there, so it has a number
    std::int64_t number = 0;  // in the measure's unit
    bool in_line = false;     // at most A18.1's line
    bool expected = false;    // within the estimate
    std::string then;         // the decision it makes
};

/// Implements PLT-04, see A18.1: the number each scene's readings, one for each variant, decide by, and its decision:
/// its deciding variant's time, or the least of its least_of's, less its minus's; for gpu_per_triangle, the slope of
/// the graphics chip's time over the triangles of the variants without the shadow pass, so the main pass alone; for
/// cpu_per_figure, the slope of the main thread's time over the figures drawn by Godot's own skeletons; each slope from
/// two variants at least. A scene with no readings was not run; a number whose readings the phone did not give is not
/// read. The scenes pass check_calibrations.
[[nodiscard]] std::vector<CalibrationVerdict> calibration_verdicts(
    const std::vector<CalibrationScene>& scenes, const std::vector<std::vector<CalibrationReading>>& readings);

/// Implements PLT-04, see A18.1: the readings of the scenes run as one code: the layout's version, the app's build and
/// which scenes ran, then each scene run in the order given, each of its variants in order, a reading the phone did
/// not give as "not measured". A scene not run has no readings.
[[nodiscard]] std::string calibration_code(std::int64_t build, const std::vector<CalibrationScene>& scenes,
                                           const std::vector<std::vector<CalibrationReading>>& readings);

/// A code read back: the build and the readings, none for a scene not run, or why it cannot be read.
struct CalibrationCodeRead {
    std::int64_t build = 0;
    std::vector<std::vector<CalibrationReading>> readings;
    std::string why;
};

/// Implements PLT-04, see A18.1: a calibration code read against the scenes it was written for, of this layout or of
/// α2.2a's, which held every scene of its build.
[[nodiscard]] CalibrationCodeRead read_calibration_code(std::string_view code,
                                                        const std::vector<CalibrationScene>& scenes);

/// Scenes read from their files: them, in the order of their names, and what is wrong, in words.
struct CalibrationSet {
    std::vector<CalibrationScene> scenes;
    std::vector<std::string> problems;
};

/// Implements PLT-04 and RES-09, see A18.1: the scenes of a set of files, in the order of their names, the one order
/// the phone runs them in and the code holds them in. A file with a problem is left out and its problems named, and
/// so are the set's own (check_calibrations).
[[nodiscard]] CalibrationSet read_calibrations(const std::vector<data::SourceFile>& files);

/// Implements PLT-04, see A18.1: a variant's readings in words, the same on the phone and in the cloud: "msaa2: the
/// graphics chip 1.23 ms, the main thread 0.45 ms, 99.8% of frames on time, 3.21 W, heat 0.45".
[[nodiscard]] std::string reading_words(const CalibrationVariant& variant, const CalibrationReading& reading);

/// Implements PLT-04, see A18.1: a scene's verdict in words: its number against its line and its estimate, and the
/// decision it makes.
[[nodiscard]] std::string verdict_words(const CalibrationScene& scene, const CalibrationVerdict& verdict);

}  // namespace kd::look
