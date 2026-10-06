#include "kd/num/letters.hpp"

#include <algorithm>

namespace kd::num {

namespace {

// Crockford's base32: the digits, then the letters but I, L, O and U.
constexpr std::string_view kAlphabet = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
// Letters in a group, the groups joined by dashes.
constexpr std::size_t kGroup = 5;
constexpr unsigned kChecksumBits = 24;

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

}  // namespace

void put_bits(std::vector<bool>& bits, std::uint64_t value, unsigned width) {
    for (unsigned i = width; i-- > 0;) {
        bits.push_back(((value >> i) & 1U) != 0);
    }
}

std::uint64_t take_bits(const std::vector<bool>& bits, std::size_t& at, unsigned width) {
    std::uint64_t v = 0;
    for (unsigned i = 0; i < width; ++i) {
        v = (v << 1U) | (bits[at] ? 1U : 0U);
        ++at;
    }
    return v;
}

std::string write_letters(std::vector<bool> bits) {
    put_bits(bits, crc24(bits), kChecksumBits);
    while (bits.size() % 5 != 0) {
        bits.push_back(false);
    }
    std::string out;
    std::size_t at = 0;
    for (std::size_t letter = 0; at < bits.size(); ++letter) {
        if (letter > 0 && letter % kGroup == 0) {
            out += '-';
        }
        out += kAlphabet[take_bits(bits, at, 5)];
    }
    return out;
}

Letters read_letters(std::string_view code, std::size_t body_bits) {
    Letters out;
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
        put_bits(bits, static_cast<std::uint64_t>(v), 5);
    }
    const std::size_t needed = body_bits + kChecksumBits;
    if (bits.size() < needed || bits.size() >= needed + 5) {
        out.wrong_length = true;
        out.why = "it has " + std::to_string(bits.size() / 5) + " letters, where a code has " +
                  std::to_string((needed + 4) / 5);
        out.body = std::move(bits);
        return out;
    }
    const std::vector<bool> body(bits.begin(), bits.begin() + static_cast<std::ptrdiff_t>(body_bits));
    std::size_t at = body_bits;
    const auto sum = static_cast<std::uint32_t>(take_bits(bits, at, kChecksumBits));
    // the padding after the checksum must be zeros, so a change to the last letter is caught too
    const bool padded =
        std::none_of(bits.begin() + static_cast<std::ptrdiff_t>(at), bits.end(), [](bool b) { return b; });
    if (sum != crc24(body) || !padded) {
        out.why = "its checksum does not hold: a letter is wrong";
        return out;
    }
    out.body = body;
    return out;
}

}  // namespace kd::num
