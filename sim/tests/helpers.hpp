// Helpers shared by the simulation's tests.
#pragma once

#include <cstdint>

namespace kd::test {

// Turns flushing of tiny numbers to zero on for this thread, as Godot's raycast module does on x86 (research 18).
inline void set_flush_to_zero() {
#if defined(__x86_64__)
    std::uint32_t mxcsr = 0;
    __asm__ volatile("stmxcsr %0" : "=m"(mxcsr));
    mxcsr |= (1U << 15) | (1U << 6);
    __asm__ volatile("ldmxcsr %0" : : "m"(mxcsr));
#else
    std::uint64_t fpcr = 0;
    __asm__ volatile("mrs %0, fpcr" : "=r"(fpcr));
    fpcr |= std::uint64_t{1} << 24;
    __asm__ volatile("msr fpcr, %0" : : "r"(fpcr));
#endif
}

}  // namespace kd::test
