#include <cstddef>
#include <cstdint>
#include <span>
#include <thread>
#include <vector>

#include "doctest.h"
#include "helpers.hpp"
#include "kd/num/digest.hpp"
#include "kd/num/fenv.hpp"

#define XXH_STATIC_LINKING_ONLY
#define XXH_INLINE_ALL
#include "xxhash.h"

// checks: RES-05
TEST_CASE("an empty digest is XXH3 of nothing") {
    kd::num::Digest d;
    CHECK(d.value() == 0x2D06800538D394C2ULL);
    CHECK(d.hex() == "2d06800538d394c2");
}

// checks: RES-05
TEST_CASE("fields are written little-endian, whatever the chip") {
    kd::num::Digest d;
    d.u32(0x01020304U);
    d.u64(0x1122334455667788ULL);
    const unsigned char expected[] = {0x04, 0x03, 0x02, 0x01, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
    CHECK(d.value() == XXH3_64bits(expected, sizeof expected));
}

// checks: RES-05
TEST_CASE("bytes and text carry their length, so moving a boundary changes the digest") {
    kd::num::Digest a;
    a.text("ab");
    a.text("c");
    kd::num::Digest b;
    b.text("a");
    b.text("bc");
    CHECK(a.value() != b.value());
}

// checks: RES-05
TEST_CASE("a long stream gives the same digest as one call over the same bytes") {
    kd::num::Digest d;
    std::vector<unsigned char> expected;
    for (std::uint64_t i = 0; i < 10'000; ++i) {
        d.u8(static_cast<std::uint8_t>(i * 7));
        expected.push_back(static_cast<unsigned char>(i * 7));
    }
    CHECK(d.value() == XXH3_64bits(expected.data(), expected.size()));
}

// checks: RES-05
TEST_CASE("a double is written as its bits") {
    kd::num::Digest a;
    a.f64(0.1);
    kd::num::Digest b;
    b.u64(0x3FB999999999999AULL);
    CHECK(a.value() == b.value());
}

// checks: RES-05
TEST_CASE("the guard sees flushing to zero, and resetting restores the default") {
    bool default_at_start = false;
    bool default_with_flush = true;
    bool default_after_reset = false;
    std::thread t([&] {
        kd::num::fenv_reset();
        default_at_start = kd::num::fenv_is_default(kd::num::fenv_read());
        kd::test::set_flush_to_zero();
        default_with_flush = kd::num::fenv_is_default(kd::num::fenv_read());
        kd::num::fenv_reset();
        default_after_reset = kd::num::fenv_is_default(kd::num::fenv_read());
    });
    t.join();
    CHECK(default_at_start);
    CHECK_FALSE(default_with_flush);
    CHECK(default_after_reset);
}
