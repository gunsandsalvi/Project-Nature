// The benchmark's result code (A18.1, PLT-04): what the phone measured, short enough to copy into a chat. The layout's
// version, then each of its fields in order, each a whole number in its own number of bits, then a CRC-24 of all
// before it, written in Crockford's base32 in groups of five letters joined by dashes. It reads back whatever its
// letters' case, wherever its lines break and whatever dashes join it, and one wrong letter is always caught
//. The layout is written once, here, for the app that writes codes and the cloud that reads them.
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace kd::bench {

/// How a field holds its measure: as it is, as a whole count, in thousandths or hundredths of one, as 16 times its
/// base-2 logarithm, or in steps of 25 (such as a clock in MHz). Every kind but "as it is" keeps 0 for "not measured".
enum class Kind : std::uint8_t { as_is, count, thousandths, hundredths, log2, per25 };

/// One field of the code: its name, such as "top.on_time", how many bits it takes and how it holds its measure.
struct Field {
    std::string name;
    unsigned bits = 0;
    Kind kind = Kind::count;
};

/// The layout's version, the code's first eight bits: a new one whenever the fields or the scenarios change, since
/// the cloud reads a code against its scenarios' digests. Version 1 is α1.5b's first build, 4de8ea2; version 3 adds
/// the graphics engine's readings (α2.1a).
inline constexpr std::uint64_t kLayoutVersion = 3;

/// The fields of this version's layout, in order: the phone's, then each scenario's measures (A18.1).
[[nodiscard]] const std::vector<Field>& layout();

/// Measures by field name, in their own units: a share as a fraction, a speed in game seconds a real second, times
/// in milliseconds. A field with none is written as "not measured"; a measure is held within its field's range.
using Values = std::map<std::string, double>;

/// Implements PLT-04, see A18.1: the code for these measures.
[[nodiscard]] std::string encode(const Values& values);

/// A code read back: its measures, those not measured left out, or why it cannot be read, in words.
struct Read {
    Values values;
    std::string why;
};

/// Implements PLT-04, see A18.1: a code read back, whatever its letters' case, its spaces and line breaks and its
/// dashes of any kind, with Crockford's look-alikes read as he says (O as 0, I and L as 1); refused if its checksum
/// does not hold or its layout is not this version's.
[[nodiscard]] Read decode(std::string_view code);

}  // namespace kd::bench
