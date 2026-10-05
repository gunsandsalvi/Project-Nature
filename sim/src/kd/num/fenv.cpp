#include "kd/num/fenv.hpp"

#include <cfenv>

#include "kd/core/check.hpp"

namespace kd::num {

std::uint64_t fenv_read() {
#if defined(__x86_64__)
    std::uint32_t mxcsr = 0;
    __asm__ volatile("stmxcsr %0" : "=m"(mxcsr));
    return mxcsr;
#elif defined(__aarch64__)
    std::uint64_t fpcr = 0;
    __asm__ volatile("mrs %0, fpcr" : "=r"(fpcr));
    return fpcr;
#else
#error "Kindling's simulation runs on x86-64 and arm64 only"
#endif
}

bool fenv_is_default(std::uint64_t value) {
#if defined(__x86_64__)
    return (value & ~std::uint64_t{0x3F}) == 0x1F80;
#else
    return value == 0;
#endif
}

void fenv_reset() {
    std::fesetenv(FE_DFL_ENV);
}

void fenv_assert_default() {
    KD_CHECK(fenv_is_default(fenv_read()),
             "a simulation thread runs in a floating-point environment that is not the default");
}

}  // namespace kd::num
