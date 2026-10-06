// A checksum of the simulation's state (A3.4): XXH3 over a canonical stream of fields, each written little-endian
// in a fixed order, never as raw memory, so the same state gives the same digest on every build and on the phone.
#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace kd::num {

/// Implements RES-05, see A3.4: the digest every same-bits comparison reads, the phone's self-check included.
class Digest {
public:
    Digest();
    Digest(const Digest&) = delete;
    Digest& operator=(const Digest&) = delete;

    void u8(std::uint8_t v);
    void u16(std::uint16_t v);
    void u32(std::uint32_t v);
    void u64(std::uint64_t v);
    void i64(std::int64_t v);
    /// A double's bits; it must be finite, since NaN's bits differ between chips.
    void f64(double v);
    /// Bytes as they are, preceded by their count.
    void bytes(std::span<const std::byte> data);
    void text(std::string_view s);
    /// Bytes as they are, with no count, so a stream fed in pieces gives the same digest however it is cut, as a
    /// file read and written in pieces must (A3.7).
    void stream(std::span<const std::byte> data);
    /// Starts again, as a new digest.
    void reset();

    /// The digest of everything written so far; writing may go on after it.
    [[nodiscard]] std::uint64_t value();
    /// The same as 16 lower-case hexadecimal digits.
    [[nodiscard]] std::string hex();

private:
    void put(const unsigned char* p, std::size_t n);
    void flush();

    alignas(64) unsigned char state_[576];
    unsigned char buffer_[256];
    std::size_t used_ = 0;
};

/// A 64-bit value as 16 lower-case hexadecimal digits.
std::string to_hex(std::uint64_t v);

}  // namespace kd::num
