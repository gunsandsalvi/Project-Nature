// Schema once (A3.6): each kind of catalogue entry names its fields in one visit() function, with their keys, types,
// units, ranges, whether they are required, what they link to and what they affect, and every reader of entries
// walks that one description: the loader, the schema writer, the fingerprinter and the display. So nothing ever
// describes a kind twice (PRN-14).
//
// A kind is a struct with a static visit, called with the entry itself, or a const one for the readers that only look:
//
//     template <typename V, typename Self>
//     static void visit(V& v, Self& m) {
//         v.quantity({"speed", "how fast it walks", Affects::rules}, m.speed, Measure::speed, {100, 3'000});
//     }
#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace kd::data {

/// What a field's value decides, and so which of its source's digests it counts in (A3.6): the rules of the world,
/// what makes the land, or only how things look.
enum class Affects : std::uint8_t { rules, world, look };

/// A field as its kind names it.
struct Field {
    std::string_view key;
    /// What the field means, in plain words, for the schema and for whoever writes entries.
    std::string_view about;
    Affects affects = Affects::rules;
    bool required = true;
};

/// An inclusive range of whole numbers, in the field's base units.
struct Range {
    std::int64_t lowest = INT64_MIN;
    std::int64_t highest = INT64_MAX;
};

/// A link from one entry to another, by name as written ("walker", or "base:walker" from another source), where it
/// was written, and once the catalogue is loaded, the linked entry's number in its kind.
struct Ref {
    std::string name;
    std::uint32_t index = 0;
    int line = 0;
    int column = 0;
};

}  // namespace kd::data
