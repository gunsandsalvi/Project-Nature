#include "kd/bench/code.hpp"

#include <algorithm>
#include <array>

#include "kd/bench/scenarios.hpp"
#include "kd/num/convert.hpp"
#include "kd/num/maths.hpp"

namespace kd::bench {

namespace {

// Crockford's base32: the digits, then the letters but I, L, O and U.
constexpr std::string_view kAlphabet = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
// Letters in a group, the groups joined by dashes.
constexpr std::size_t kGroup = 5;
constexpr unsigned kVersionBits = 8;
constexpr unsigned kChecksumBits = 24;

struct Measure {
    std::string_view name;
    unsigned bits;
    Kind kind;
};

// The phone's own fields.
constexpr std::array<Measure, 9> kPhone{{
    {"build", 16, Kind::as_is},      // the app's version code, such as 20502
    {"cores", 5, Kind::as_is},       // the processor's cores
    {"big_mhz", 8, Kind::per25},     // the fastest core's top clock
    {"refresh_hz", 9, Kind::count},  // the screen's refresh rate
    {"android", 7, Kind::count},     // the Android version, such as 16
    {"battery", 7, Kind::count},     // the battery's charge at the start, in percent
    {"plugged", 2, Kind::as_is},     // 1 on battery, 2 plugged in
    {"thermal", 2, Kind::as_is},     // 1 when the heat forecast works, 2 when it does not
    {"seconds", 12, Kind::count},    // the real seconds the whole run took
}};

// Each scenario's (A18.1).
constexpr std::array<Measure, 13> kEach{{
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
}};

// What only a scenario that saves measures, in milliseconds: its slowest save's pause, its export, its reopening.
constexpr std::array<Measure, 3> kSaves{{
    {"save_ms", 11, Kind::count},
    {"export_ms", 15, Kind::count},
    {"open_ms", 15, Kind::count},
}};

// CRC-24 as OpenPGP defines it, over whole bits, most significant first. It catches every change within 24 bits in a
// row, so every wrong letter.
std::uint32_t crc24(const std::vector<bool>& bits) {
    std::uint32_t crc = 0xB704CEU;
    for (const bool b : bits) {
        const bool top = ((crc >> 23U) & 1U) != 0;
        crc = (crc << 1U) & 0xFFFFFFU;
        if (top != b) {
            crc ^= 0x864CFBU;
        }
    }
    return crc;
}

void put(std::vector<bool>& bits, std::uint64_t value, unsigned width) {
    for (unsigned i = width; i-- > 0;) {
        bits.push_back(((value >> i) & 1U) != 0);
    }
}

std::uint64_t take(const std::vector<bool>& bits, std::size_t& at, unsigned width) {
    std::uint64_t v = 0;
    for (unsigned i = 0; i < width; ++i) {
        v = (v << 1U) | (bits[at] ? 1U : 0U);
        ++at;
    }
    return v;
}

// A letter's value, read as Crockford says: either case, O as 0, I and L as 1; -1 for any other.
int value_of(char c) {
    if (c >= 'a' && c <= 'z') {
        c = static_cast<char>(c - 'a' + 'A');
    }
    if (c == 'O') {
        return 0;
    }
    if (c == 'I' || c == 'L') {
        return 1;
    }
    const std::size_t at = kAlphabet.find(c);
    return at == std::string_view::npos ? -1 : static_cast<int>(at);
}

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
    put(bits, kLayoutVersion, kVersionBits);
    for (const Field& f : layout()) {
        const auto it = values.find(f.name);
        put(bits, it == values.end() ? 0 : stored(f, it->second), f.bits);
    }
    put(bits, crc24(bits), kChecksumBits);
    while (bits.size() % 5 != 0) {
        bits.push_back(false);
    }
    std::string out;
    std::size_t at = 0;
    for (std::size_t letter = 0; at < bits.size(); ++letter) {
        if (letter > 0 && letter % kGroup == 0) {
            out += '-';
        }
        out += kAlphabet[take(bits, at, 5)];
    }
    return out;
}

Read decode(std::string_view code) {
    Read out;
    std::vector<bool> bits;
    for (const char c : code) {
        // spaces, line breaks, hyphens, and the other dashes and spaces chat apps swap in, whose UTF-8 bytes are
        // all above 127, are skipped
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '-' || static_cast<unsigned char>(c) >= 0x80) {
            continue;
        }
        const int v = value_of(c);
        if (v < 0) {
            out.why = std::string("it holds a letter no code has: ") + c;
            return out;
        }
        put(bits, static_cast<std::uint64_t>(v), 5);
    }
    std::size_t needed = kVersionBits + kChecksumBits;
    for (const Field& f : layout()) {
        needed += f.bits;
    }
    if (bits.size() < needed || bits.size() >= needed + 5) {
        std::size_t at = 0;
        const std::uint64_t version = bits.size() >= kVersionBits ? take(bits, at, kVersionBits) : 0;
        out.why = version != kLayoutVersion && bits.size() >= kVersionBits
                      ? "it is a code of layout " + std::to_string(version) + ", and this reads layout " +
                            std::to_string(kLayoutVersion)
                      : "it has " + std::to_string(bits.size() / 5) + " letters, where a code has " +
                            std::to_string((needed + 4) / 5);
        return out;
    }
    const std::vector<bool> body(bits.begin(), bits.begin() + static_cast<std::ptrdiff_t>(needed - kChecksumBits));
    std::size_t at = body.size();
    const auto sum = static_cast<std::uint32_t>(take(bits, at, kChecksumBits));
    // the padding after the checksum must be zeros, so a change to the last letter is caught too
    const bool padded =
        std::none_of(bits.begin() + static_cast<std::ptrdiff_t>(at), bits.end(), [](bool b) { return b; });
    if (sum != crc24(body) || !padded) {
        out.why = "its checksum does not hold: a letter is wrong";
        return out;
    }
    at = 0;
    const std::uint64_t version = take(bits, at, kVersionBits);
    if (version != kLayoutVersion) {
        out.why = "it is a code of layout " + std::to_string(version) + ", and this reads layout " +
                  std::to_string(kLayoutVersion);
        return out;
    }
    for (const Field& f : layout()) {
        if (const std::optional<double> m = measure(f, take(bits, at, f.bits))) {
            out.values[f.name] = *m;
        }
    }
    return out;
}

}  // namespace kd::bench
