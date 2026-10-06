#include "kd/num/digest.hpp"

#include <bit>
#include <cmath>
#include <cstring>

#include "kd/core/check.hpp"

#define XXH_STATIC_LINKING_ONLY
#define XXH_INLINE_ALL
#include "xxhash.h"

namespace kd::num {

static_assert(sizeof(XXH3_state_t) <= sizeof(unsigned char[576]), "the digest's state no longer fits");
static_assert(alignof(XXH3_state_t) <= 64);

namespace {

XXH3_state_t* state_of(unsigned char* storage) {
    return reinterpret_cast<XXH3_state_t*>(storage);
}

void little(unsigned char* out, std::uint64_t v, int n) {
    for (int i = 0; i < n; ++i) {
        out[i] = static_cast<unsigned char>(v >> (8 * i));
    }
}

}  // namespace

Digest::Digest() {
    KD_CHECK(XXH3_64bits_reset(state_of(state_)) == XXH_OK, "the digest could not start");
}

void Digest::put(const unsigned char* p, std::size_t n) {
    // nothing to take, as from an empty span, whose pointer may be null
    if (n == 0) {
        return;
    }
    if (used_ + n > sizeof buffer_) {
        flush();
    }
    if (n > sizeof buffer_) {
        KD_CHECK(XXH3_64bits_update(state_of(state_), p, n) == XXH_OK, "the digest could not take its input");
        return;
    }
    std::memcpy(buffer_ + used_, p, n);
    used_ += n;
}

void Digest::flush() {
    if (used_ > 0) {
        KD_CHECK(XXH3_64bits_update(state_of(state_), buffer_, used_) == XXH_OK, "the digest could not take its input");
        used_ = 0;
    }
}

void Digest::u8(std::uint8_t v) {
    put(&v, 1);
}

void Digest::u16(std::uint16_t v) {
    unsigned char b[2];
    little(b, v, 2);
    put(b, 2);
}

void Digest::u32(std::uint32_t v) {
    unsigned char b[4];
    little(b, v, 4);
    put(b, 4);
}

void Digest::u64(std::uint64_t v) {
    unsigned char b[8];
    little(b, v, 8);
    put(b, 8);
}

void Digest::i64(std::int64_t v) {
    u64(static_cast<std::uint64_t>(v));
}

void Digest::f64(double v) {
    KD_CHECK(std::isfinite(v), "a number that is not finite reached the simulation's state");
    u64(std::bit_cast<std::uint64_t>(v));
}

void Digest::bytes(std::span<const std::byte> data) {
    u64(data.size());
    put(reinterpret_cast<const unsigned char*>(data.data()), data.size());
}

void Digest::stream(std::span<const std::byte> data) {
    put(reinterpret_cast<const unsigned char*>(data.data()), data.size());
}

void Digest::reset() {
    used_ = 0;
    KD_CHECK(XXH3_64bits_reset(state_of(state_)) == XXH_OK, "the digest could not start");
}

void Digest::text(std::string_view s) {
    bytes(std::as_bytes(std::span<const char>(s.data(), s.size())));
}

std::uint64_t Digest::value() {
    flush();
    return XXH3_64bits_digest(state_of(state_));
}

std::string Digest::hex() {
    return to_hex(value());
}

std::string to_hex(std::uint64_t v) {
    static constexpr char digits[] = "0123456789abcdef";
    std::string out(16, '0');
    for (int i = 15; i >= 0; --i) {
        out[static_cast<std::size_t>(i)] = digits[v & 15U];
        v >>= 4;
    }
    return out;
}

}  // namespace kd::num
