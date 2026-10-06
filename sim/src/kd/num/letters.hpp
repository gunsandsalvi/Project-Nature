// Short codes to copy into a chat (A18.1, PLT-04): whole numbers packed as bits, then a CRC-24 of them, written in
// Crockford's base32 in groups of five letters joined by dashes. A code reads back whatever its letters' case,
// wherever its lines break and whatever dashes join it, and one wrong letter is always caught. The benchmark's code
// and the blind test's are written and read here, once.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace kd::num {

/// Adds a whole number to bits, in width bits, most significant first.
void put_bits(std::vector<bool>& bits, std::uint64_t value, unsigned width);

/// Takes a whole number of width bits from bits at at, moving at past it.
[[nodiscard]] std::uint64_t take_bits(const std::vector<bool>& bits, std::size_t& at, unsigned width);

/// Implements PLT-04, see A18.1: bits as a code: their 24-bit checksum after them, padded with zeros to whole letters.
[[nodiscard]] std::string write_letters(std::vector<bool> bits);

/// A code's letters read back.
struct Letters {
    std::vector<bool> body;     // the bits before the checksum, when it reads
    std::string why;            // why it does not read, in words; empty when it does
    bool wrong_length = false;  // it is not the body's length and the checksum's; body then holds every bit read
};

/// Implements PLT-04, see A18.1: a code of body_bits bits before its checksum, read back whatever its letters' case,
/// its spaces and line breaks and its dashes of any kind, with Crockford's look-alikes read as he says (O as 0, I and
/// L as 1); refused if a letter is no code's, if it is not the body's length and the checksum's, or if its checksum
/// does not hold, its padding included.
[[nodiscard]] Letters read_letters(std::string_view code, std::size_t body_bits);

}  // namespace kd::num
