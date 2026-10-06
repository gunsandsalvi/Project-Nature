#include "kd/bench/code.hpp"

#include <algorithm>
#include <array>

#include "kd/bench/scenarios.hpp"
#include "kd/num/convert.hpp"
#include "kd/num/letters.hpp"
#include "kd/num/maths.hpp"

namespace kd::bench {

namespace {

constexpr unsigned kVersionBits = 8;

struct Measure {
    std::string_view name;
    unsigned bits;
    Kind kind;
};

// The phone's own fields.
constexpr std::array<Measure, 12> kPhone{{
    {"build", 16, Kind::as_is},      // the app's version code, such as 20502
    {"cores", 5, Kind::as_is},       // the processor's cores
    {"big_mhz", 8, Kind::per25},     // the fastest core's top clock
    {"refresh_hz", 9, Kind::count},  // the screen's refresh rate
    {"android", 7, Kind::count},     // the Android API level, such as 36
    {"battery", 7, Kind::count},     // the battery's charge at the start, in percent
    {"plugged", 2, Kind::as_is},     // 1 on battery, 2 plugged in
    {"thermal", 2, Kind::as_is},     // 1 when the heat forecast works, 2 when it does not
    {"seconds", 12, Kind::count},    // the real seconds the whole run took
    // from layout 3 (α2.1a): the heat at which the phone's light and moderate throttling begin, as Android's
    // headroom (Android 15), and whether it gives the graphics chip's headroom (Android 16): 1 yes, 2 no
    {"light", 8, Kind::hundredths},
    {"moderate", 8, Kind::hundredths},
    {"gpu_offered", 2, Kind::as_is},
}};

// Each scenario's (A18.1).
constexpr std::array<Measure, 20> kEach{{
    {"on_time", 10, Kind::thousandths},  // the share of frames on time
    {"slowest", 11, Kind::count},        // the slowest frame, in milliseconds
    {"stalls", 13, Kind::count},         // frame periods skipped, in all
    {"speed", 10, Kind::log2},           // the speed held, in game seconds a real second
    {"digest", 2, Kind::as_is},          // 1 when its digest is the cloud's, 2 when it differs
    {"digest_bits", 20, Kind::as_is},    // its digest's top 20 bits
    {"heat", 9, Kind::hundredths},       // the highest heat forecast, as a share of the first throttling level
    {"share", 7, Kind::count},           // the lowest working share the heat allowed, in percent
    {"current", 13, Kind::count},        // the battery's mean current drawn, in milliamperes
    {"memory", 13, Kind::count},         // the most memory the app held, in megabytes
    {"cpu", 8, Kind::count},             // the world's thread's share of one core, in percent
    {"draw_ms", 11, Kind::hundredths},   // the crowd's drawing, its mean milliseconds of the main thread a frame
    {"clock", 8, Kind::per25},           // the fastest core's mean clock in MHz, which falls as the phone throttles
    // from layout 3 (α2.1a), for the graphics engine (A18.1)
    {"gpu_ms", 12, Kind::hundredths},  // the graphics chip's mean milliseconds a frame for the whole window
    {"gpu_headroom", 8, Kind::count},  // the graphics chip's least headroom, from 0 to 100, where Android gives it
    {"power", 12, Kind::hundredths},   // the phone's mean power drawn from the battery, in watts
    {"draws", 13, Kind::count},        // the most draw calls in a frame
    {"triangles", 12, Kind::count},    // the most triangles in a frame, in thousands
    {"video_mb", 12, Kind::count},     // the most video memory used, in megabytes
    {"to_light", 9, Kind::count},      // minutes to the light throttling level at the rate the heat rose; 500 if never
}};

// What only a scenario that saves measures, in milliseconds: its slowest save's pause, its export, its reopening.
constexpr std::array<Measure, 3> kSaves{{
    {"save_ms", 11, Kind::count},
    {"export_ms", 15, Kind::count},
    {"open_ms", 15, Kind::count},
}};

// A measure as its field holds it, within the field's bits.
std::uint64_t stored(const Field& f, double x) {
    const std::int64_t most = (std::int64_t{1} << f.bits) - 1;
    const auto whole = [&](double v) { return std::clamp<std::int64_t>(num::to_int(v, num::Round::nearest), 0, most); };
    switch (f.kind) {
        case Kind::as_is:
            return static_cast<std::uint64_t>(whole(std::max(x, 0.0)));
        case Kind::count:
            return static_cast<std::uint64_t>(std::min(whole(std::max(x, 0.0)) + 1, most));
        case Kind::thousandths:
            return static_cast<std::uint64_t>(std::min(whole(std::max(x, 0.0) * 1000.0) + 1, most));
        case Kind::hundredths:
            return static_cast<std::uint64_t>(std::min(whole(std::max(x, 0.0) * 100.0) + 1, most));
        case Kind::log2:
            return static_cast<std::uint64_t>(std::min(whole(16.0 * num::log2(std::max(x, 1.0))) + 1, most));
        case Kind::per25:
            return static_cast<std::uint64_t>(std::min(whole(std::max(x, 0.0) / 25.0) + 1, most));
    }
    return 0;
}

// A field's measure from what it holds; nothing for "not measured".
std::optional<double> measure(const Field& f, std::uint64_t v) {
    if (f.kind == Kind::as_is) {
        return static_cast<double>(v);
    }
    if (v == 0) {
        return std::nullopt;
    }
    const auto n = static_cast<double>(v - 1);
    switch (f.kind) {
        case Kind::count:
            return n;
        case Kind::thousandths:
            return n / 1000.0;
        case Kind::hundredths:
            return n / 100.0;
        case Kind::log2:
            return num::exp2(n / 16.0);
        case Kind::per25:
            return n * 25.0;
        case Kind::as_is:
            break;
    }
    return std::nullopt;
}

}  // namespace

const std::vector<Field>& layout() {
    static const std::vector<Field> fields = [] {
        std::vector<Field> out;
        out.reserve(kPhone.size() + scenarios().size() * kEach.size() + kSaves.size());
        for (const Measure& m : kPhone) {
            out.push_back({std::string(m.name), m.bits, m.kind});
        }
        for (const Scenario& s : scenarios()) {
            for (const Measure& m : kEach) {
                out.push_back({std::string(s.name) + "." + std::string(m.name), m.bits, m.kind});
            }
            if (s.saves) {
                for (const Measure& m : kSaves) {
                    out.push_back({std::string(s.name) + "." + std::string(m.name), m.bits, m.kind});
                }
            }
        }
        return out;
    }();
    return fields;
}

std::string encode(const Values& values) {
    std::vector<bool> bits;
    num::put_bits(bits, kLayoutVersion, kVersionBits);
    for (const Field& f : layout()) {
        const auto it = values.find(f.name);
        num::put_bits(bits, it == values.end() ? 0 : stored(f, it->second), f.bits);
    }
    return num::write_letters(std::move(bits));
}

Read decode(std::string_view code) {
    Read out;
    std::size_t needed = kVersionBits;
    for (const Field& f : layout()) {
        needed += f.bits;
    }
    const num::Letters letters = num::read_letters(code, needed);
    std::size_t at = 0;
    if (letters.wrong_length && letters.body.size() >= kVersionBits) {
        // a code of another layout has another length: name its layout
        const std::uint64_t version = num::take_bits(letters.body, at, kVersionBits);
        if (version != kLayoutVersion) {
            out.why = "it is a code of layout " + std::to_string(version) + ", and this reads layout " +
                      std::to_string(kLayoutVersion);
            return out;
        }
    }
    if (!letters.why.empty()) {
        out.why = letters.why;
        return out;
    }
    at = 0;
    const std::uint64_t version = num::take_bits(letters.body, at, kVersionBits);
    if (version != kLayoutVersion) {
        out.why = "it is a code of layout " + std::to_string(version) + ", and this reads layout " +
                  std::to_string(kLayoutVersion);
        return out;
    }
    for (const Field& f : layout()) {
        if (const std::optional<double> m = measure(f, num::take_bits(letters.body, at, f.bits))) {
            out.values[f.name] = *m;
        }
    }
    return out;
}

}  // namespace kd::bench
