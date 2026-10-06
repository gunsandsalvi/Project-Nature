#include "kd/look/calibration.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "kd/core/check.hpp"
#include "kd/data/loader.hpp"
#include "kd/num/convert.hpp"
#include "kd/num/letters.hpp"
#include "kd/scene/scene.hpp"

namespace kd::look {

namespace {

constexpr std::array<std::string_view, 4> kDraws{"nothing", "field", "rocks", "copies"};
constexpr std::array<std::string_view, 4> kPaths{"still", "pan", "turn", "pinch"};
constexpr std::array<std::string_view, 3> kMeasures{"gpu", "cpu", "gpu_per_triangle"};

constexpr std::uint64_t kLayoutVersion = 1;
constexpr unsigned kVersionBits = 8;
constexpr unsigned kBuildBits = 17;

// Each reading's bits in the code, in the order a reading lists them. A field holds its reading plus one, and 0 for
// "not measured", as the benchmark's code does.
struct Width {
    std::int64_t CalibrationReading::*field;
    unsigned bits;
};
constexpr std::array<Width, 5> kWidths{{
    {&CalibrationReading::gpu_us, 16},
    {&CalibrationReading::cpu_us, 16},
    {&CalibrationReading::on_time, 11},
    {&CalibrationReading::power_mw, 15},
    {&CalibrationReading::heat, 9},
}};

std::size_t variants_in(const std::vector<CalibrationScene>& scenes) {
    std::size_t n = 0;
    for (const CalibrationScene& s : scenes) {
        n += s.variants.size();
    }
    return n;
}

std::size_t body_bits(const std::vector<CalibrationScene>& scenes) {
    std::size_t per = 0;
    for (const Width& w : kWidths) {
        per += w.bits;
    }
    return kVersionBits + kBuildBits + per * variants_in(scenes);
}

void read_variant(const data::Value& table, const std::string& path, std::vector<data::Problem>& problems,
                  CalibrationVariant& v) {
    data::Loader l(table, path, problems);
    using data::Affects;
    l.text({"name", "the variant's name, as the code and the cloud's reading name it"}, v.name);
    l.whole({"msaa", "MSAA's samples: 0, 2 or 4", Affects::look, false}, v.msaa, {0, 4});
    l.whole({"scale", "the 3D's scale, in percent of the screen's resolution", Affects::look, false}, v.scale,
            {25, 100});
    l.truth({"interface", "the app's interface drawn over the world", Affects::look, false}, v.interface);
    l.truth({"shadows", "the sun's shadow pass", Affects::look, false}, v.shadows);
    l.whole({"triangles", "for rocks: thousands of triangles a pass", Affects::look, false}, v.triangles, {0, 4'000});
    l.whole({"copies", "for copies: draws of copies", Affects::look, false}, v.copies, {0, 4'000});
    l.whole({"passes", "for copies: 2, the main pass and the sun's shadow, or 3 with a mirror's pass", Affects::look,
             false},
            v.passes, {2, 3});
    l.finish();
    if (v.msaa != 0 && v.msaa != 2 && v.msaa != 4) {
        l.refuse(*table.find("msaa"), "msaa: MSAA takes 0, 2 or 4 samples");
    }
}

}  // namespace

CalibrationRead read_calibration(std::string_view text, const std::string& path) {
    CalibrationRead out;
    CalibrationScene& s = out.scene;
    s.name = scene::name_of(path);
    data::Parsed parsed = data::parse_toml(text, path);
    out.problems = std::move(parsed.problems);
    if (!out.problems.empty()) {
        return out;
    }
    data::Loader l(parsed.root, path, out.problems);
    l.text({"about", "what the scene measures, in a sentence"}, s.about);
    l.names({"checks", "the items it serves, by ID, such as PLT-04"}, s.checks);
    l.choice({"draws", "what it draws: nothing, field, rocks or copies"}, s.draws, kDraws);
    l.choice({"path", "the camera: still, pan, turn or pinch"}, s.path, kPaths);
    l.choice({"measure",
              "the number that decides: gpu or cpu in microseconds a frame, or gpu_per_triangle in "
              "picoseconds"},
             s.measure, kMeasures);
    l.text({"decides", "for gpu and cpu: the variant whose number decides", data::Affects::rules, false}, s.decides);
    l.text({"minus", "for gpu and cpu: \"scene/variant\", whose same number is taken off", data::Affects::rules, false},
           s.minus);
    l.whole({"line", "A18.1's allowance, in the measure's unit"}, s.line, {1, 1'000'000});
    l.whole({"expect_from", "the estimate's low end, in the measure's unit"}, s.expect_from, {0, 1'000'000});
    l.whole({"expect_to", "the estimate's high end, in the measure's unit"}, s.expect_to, {0, 1'000'000});
    const std::vector<const data::Value*> variants = l.tables({"variant", "the variants, each with its switches"});
    const std::vector<const data::Value*> decisions =
        l.tables({"decision", "the decisions, by the number at most, the last for any number"});
    l.finish();

    for (const std::string& id : s.checks) {
        if (!scene::item_id(id)) {
            l.refuse(*parsed.root.find("checks"), "checks: \"" + id + "\" is not an item's ID, such as PLT-04");
        }
    }
    for (const data::Value* v : variants) {
        CalibrationVariant variant;
        read_variant(*v, path, out.problems, variant);
        s.variants.push_back(variant);
    }
    for (const data::Value* d : decisions) {
        data::Loader dl(*d, path, out.problems);
        CalibrationDecision decision;
        std::int64_t most = 0;
        dl.whole({"at_most", "the number at most this, in the measure's unit; none on the last", data::Affects::rules,
                  false},
                 most, {0, 1'000'000});
        dl.text({"then", "what the plan does then"}, decision.then);
        dl.finish();
        if (d->find("at_most") != nullptr) {
            decision.at_most = most;
        }
        s.decisions.push_back(decision);
    }
    if (!out.problems.empty()) {
        return out;
    }
    // the decisions rise and the last takes every number, so each number makes exactly one
    for (std::size_t i = 0; i < s.decisions.size(); ++i) {
        const bool last = i + 1 == s.decisions.size();
        const std::optional<std::int64_t>& most = s.decisions[i].at_most;
        const std::optional<std::int64_t> before = i > 0 ? s.decisions[i - 1].at_most : std::nullopt;
        if (last == most.has_value()) {
            l.refuse(*decisions[i], last ? "decision: the last takes every number, so has no at_most"
                                         : "decision: every decision but the last has an at_most");
        } else if (most && before && *most <= *before) {
            l.refuse(*decisions[i], "decision: each at_most is above the one before");
        }
    }
    if (s.checks.empty()) {
        l.refuse(*parsed.root.find("checks"), "checks: a scene names the items it serves, such as PLT-04");
    }
    if (s.decisions.empty()) {
        l.refuse(parsed.root, "decision: a scene states a decision for every number, the last with no at_most");
    }
    if (s.expect_from > s.expect_to) {
        l.refuse(*parsed.root.find("expect_to"), "expect_to: the estimate's high end is below its low end");
    }
    if (s.variants.empty()) {
        l.refuse(parsed.root, "variant: a scene has a variant at least");
    } else if (s.measure == "gpu_per_triangle") {
        const auto counted = std::count_if(s.variants.begin(), s.variants.end(),
                                           [](const CalibrationVariant& v) { return v.triangles > 0 && !v.shadows; });
        if (counted < 2) {
            l.refuse(parsed.root, "variant: gpu_per_triangle needs two variants with triangles and no shadow pass");
        }
        if (!s.decides.empty() || !s.minus.empty()) {
            l.refuse(parsed.root,
                     "decides: a slope decides by every variant without the shadow pass and takes nothing off, so "
                     "names no variant and no minus");
        }
    } else if (std::none_of(s.variants.begin(), s.variants.end(),
                            [&](const CalibrationVariant& v) { return v.name == s.decides; })) {
        l.refuse(parsed.root, "decides: \"" + s.decides + "\" names none of the scene's variants");
    }
    for (std::size_t i = 0; i < s.variants.size(); ++i) {
        const CalibrationVariant& v = s.variants[i];
        if ((s.draws == "rocks") != (v.triangles > 0) || (s.draws == "copies") != (v.copies > 0)) {
            l.refuse(*variants[i],
                     "variant: rocks give each variant its triangles, copies its copies, and no other "
                     "scene either");
        }
        for (std::size_t j = 0; j < i; ++j) {
            if (s.variants[j].name == v.name) {
                l.refuse(*variants[i], "variant: \"" + v.name + "\" is named twice");
            }
        }
    }
    return out;
}

std::vector<std::string> check_calibrations(const std::vector<CalibrationScene>& scenes) {
    std::vector<std::string> out;
    for (std::size_t i = 0; i < scenes.size(); ++i) {
        const CalibrationScene& s = scenes[i];
        for (std::size_t j = 0; j < i; ++j) {
            if (scenes[j].name == s.name) {
                out.push_back(s.name + ": two scenes have this name");
            }
        }
        if (s.minus.empty()) {
            continue;
        }
        const std::size_t slash = s.minus.find('/');
        const std::string scene = s.minus.substr(0, slash);
        const std::string variant = slash == std::string::npos ? "" : s.minus.substr(slash + 1);
        const auto other =
            std::find_if(scenes.begin(), scenes.end(), [&](const CalibrationScene& o) { return o.name == scene; });
        if (other == scenes.end() || std::none_of(other->variants.begin(), other->variants.end(),
                                                  [&](const CalibrationVariant& v) { return v.name == variant; })) {
            out.push_back(s.name + ": minus \"" + s.minus + "\" names no scene and variant here");
        }
    }
    return out;
}

std::vector<CalibrationVerdict> calibration_verdicts(const std::vector<CalibrationScene>& scenes,
                                                     const std::vector<std::vector<CalibrationReading>>& readings) {
    KD_CHECK(readings.size() == scenes.size(), "look: readings for each calibration scene");
    KD_CHECK(check_calibrations(scenes).empty(), "look: calibration scenes that pass their checks");
    // a variant's time by the measure, its scene's or another's; -1 where the phone gave none
    const auto time_of = [&](const std::string& scene, const std::string& variant, const std::string& measure) {
        for (std::size_t si = 0; si < scenes.size(); ++si) {
            for (std::size_t v = 0; v < scenes[si].variants.size(); ++v) {
                if (scenes[si].name == scene && scenes[si].variants[v].name == variant) {
                    const CalibrationReading& r = readings[si][v];
                    return measure == "gpu" ? r.gpu_us : r.cpu_us;
                }
            }
        }
        return std::int64_t{-1};
    };
    std::vector<CalibrationVerdict> out;
    for (std::size_t si = 0; si < scenes.size(); ++si) {
        const CalibrationScene& scene = scenes[si];
        KD_CHECK(readings[si].size() == scene.variants.size(),
                 "look: a reading for each of a calibration scene's variants");
        CalibrationVerdict v;
        if (scene.measure == "gpu_per_triangle") {
            // the least-squares slope of the time over the triangles, in the variants without the shadow pass whose
            // time the phone gave: microseconds a thousand triangles are nanoseconds a triangle
            std::vector<std::pair<double, double>> points;
            for (std::size_t i = 0; i < scene.variants.size(); ++i) {
                const CalibrationVariant& variant = scene.variants[i];
                if (variant.triangles > 0 && !variant.shadows && readings[si][i].gpu_us >= 0) {
                    points.emplace_back(static_cast<double>(variant.triangles),
                                        static_cast<double>(readings[si][i].gpu_us));
                }
            }
            double x_mean = 0.0;
            double y_mean = 0.0;
            for (const auto& [x, y] : points) {
                x_mean += x / static_cast<double>(points.size());
                y_mean += y / static_cast<double>(points.size());
            }
            double across = 0.0;
            double spread = 0.0;
            for (const auto& [x, y] : points) {
                across += (x - x_mean) * (y - y_mean);
                spread += (x - x_mean) * (x - x_mean);
            }
            v.read = points.size() >= 2 && spread > 0.0;
            if (v.read) {
                v.number = num::to_int(across / spread * 1000.0, num::Round::nearest);
            }
        } else {
            const std::int64_t time = time_of(scene.name, scene.decides, scene.measure);
            v.read = time >= 0;
            v.number = time;
            if (!scene.minus.empty()) {
                const std::size_t slash = scene.minus.find('/');
                const std::int64_t less =
                    time_of(scene.minus.substr(0, slash), scene.minus.substr(slash + 1), scene.measure);
                v.read = v.read && less >= 0;
                v.number -= less;
            }
        }
        if (v.read) {
            v.in_line = v.number <= scene.line;
            v.expected = v.number >= scene.expect_from && v.number <= scene.expect_to;
            for (const CalibrationDecision& d : scene.decisions) {
                if (!d.at_most || v.number <= *d.at_most) {
                    v.then = d.then;
                    break;
                }
            }
        } else {
            v.number = 0;
        }
        out.push_back(v);
    }
    return out;
}

std::string calibration_code(std::int64_t build, const std::vector<CalibrationScene>& scenes,
                             const std::vector<std::vector<CalibrationReading>>& readings) {
    KD_CHECK(readings.size() == scenes.size(), "look: readings for each calibration scene");
    std::vector<bool> bits;
    num::put_bits(bits, kLayoutVersion, kVersionBits);
    num::put_bits(bits, static_cast<std::uint64_t>(std::clamp<std::int64_t>(build, 0, (1 << kBuildBits) - 1)),
                  kBuildBits);
    for (std::size_t s = 0; s < scenes.size(); ++s) {
        KD_CHECK(readings[s].size() == scenes[s].variants.size(),
                 "look: a reading for each of a calibration scene's variants");
        for (const CalibrationReading& r : readings[s]) {
            for (const Width& w : kWidths) {
                const std::int64_t most = (std::int64_t{1} << w.bits) - 1;
                const std::int64_t held = r.*w.field < 0 ? 0 : std::min(r.*w.field + 1, most);
                num::put_bits(bits, static_cast<std::uint64_t>(held), w.bits);
            }
        }
    }
    return num::write_letters(std::move(bits));
}

CalibrationCodeRead read_calibration_code(std::string_view code, const std::vector<CalibrationScene>& scenes) {
    CalibrationCodeRead out;
    const num::Letters letters = num::read_letters(code, body_bits(scenes));
    std::size_t at = 0;
    if (letters.wrong_length && letters.body.size() >= kVersionBits) {
        const std::uint64_t version = num::take_bits(letters.body, at, kVersionBits);
        if (version != kLayoutVersion) {
            out.why = "it is a calibration code of layout " + std::to_string(version) + ", and this reads layout " +
                      std::to_string(kLayoutVersion);
            return out;
        }
        out.why = letters.why + ": the scenes it was written for differ from these";
        return out;
    }
    if (!letters.why.empty()) {
        out.why = letters.why;
        return out;
    }
    at = 0;
    const std::uint64_t version = num::take_bits(letters.body, at, kVersionBits);
    if (version != kLayoutVersion) {
        out.why = "it is a calibration code of layout " + std::to_string(version) + ", and this reads layout " +
                  std::to_string(kLayoutVersion);
        return out;
    }
    out.build = static_cast<std::int64_t>(num::take_bits(letters.body, at, kBuildBits));
    for (const CalibrationScene& s : scenes) {
        std::vector<CalibrationReading> scene_readings;
        for (std::size_t v = 0; v < s.variants.size(); ++v) {
            CalibrationReading r;
            for (const Width& w : kWidths) {
                r.*w.field = static_cast<std::int64_t>(num::take_bits(letters.body, at, w.bits)) - 1;
            }
            scene_readings.push_back(r);
        }
        out.readings.push_back(scene_readings);
    }
    return out;
}

CalibrationSet read_calibrations(const std::vector<data::SourceFile>& files) {
    CalibrationSet out;
    for (const data::SourceFile& f : files) {
        CalibrationRead read = read_calibration(f.text, f.path);
        for (const data::Problem& p : read.problems) {
            out.problems.push_back(data::problem_text(p));
        }
        if (read.problems.empty()) {
            out.scenes.push_back(std::move(read.scene));
        }
    }
    std::stable_sort(out.scenes.begin(), out.scenes.end(),
                     [](const CalibrationScene& a, const CalibrationScene& b) { return a.name < b.name; });
    for (std::string& problem : check_calibrations(out.scenes)) {
        out.problems.push_back(std::move(problem));
    }
    return out;
}

namespace {

// A number of thousandths with two decimal places, rounded half away from zero: 1235 is "1.24".
std::string two_places(std::int64_t thousandths) {
    const std::int64_t hundredths = ((thousandths < 0 ? -thousandths : thousandths) + 5) / 10;
    const std::int64_t part = hundredths % 100;
    return std::string(thousandths < 0 && hundredths > 0 ? "-" : "") + std::to_string(hundredths / 100) + "." +
           (part < 10 ? "0" : "") + std::to_string(part);
}

}  // namespace

std::string reading_words(const CalibrationVariant& variant, const CalibrationReading& reading) {
    const auto put = [](std::int64_t value, const std::string& said, const std::string& missing) {
        return value < 0 ? missing + " not measured" : said;
    };
    return variant.name + ": " +
           put(reading.gpu_us, "the graphics chip " + two_places(reading.gpu_us) + " ms", "the graphics chip") + ", " +
           put(reading.cpu_us, "the main thread " + two_places(reading.cpu_us) + " ms", "the main thread") + ", " +
           put(reading.on_time,
               std::to_string(reading.on_time / 10) + "." + std::to_string(reading.on_time % 10) +
                   "% of frames on time",
               "frames on time") +
           ", " + put(reading.power_mw, two_places(reading.power_mw) + " W", "power") + ", " +
           put(reading.heat, "heat " + two_places(reading.heat * 10), "heat");
}

std::string verdict_words(const CalibrationScene& scene, const CalibrationVerdict& verdict) {
    if (!verdict.read) {
        return scene.name + ": not measured, so it decides nothing yet";
    }
    const bool slope = scene.measure == "gpu_per_triangle";
    const std::string unit = slope ? " ns a triangle" : " ms";
    std::string what;
    if (slope) {
        what = "the graphics chip " + two_places(verdict.number) + unit + " in the main pass";
    } else {
        what = std::string(scene.measure == "gpu" ? "the graphics chip " : "the main thread ") +
               two_places(verdict.number) + unit + " for " + scene.decides +
               (scene.minus.empty() ? "" : " less " + scene.minus);
    }
    return scene.name + ": " + what + ", " + (verdict.in_line ? "within" : "over") + " its line of " +
           two_places(scene.line) + unit + ", " + (verdict.expected ? "within" : "outside") + " its estimate of " +
           two_places(scene.expect_from) + " to " + two_places(scene.expect_to) + unit + "; so: " + verdict.then;
}

}  // namespace kd::look
