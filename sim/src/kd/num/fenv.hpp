// The floating-point environment (A3.4): a new thread inherits its creator's, which may flush tiny numbers to zero
// (Godot's raycast module sets that on x86, and what the phone's graphics driver does is unknown), so every
// simulation thread sets the default first and checkpoints assert it.
#pragma once

#include <cstdint>

namespace kd::num {

/// The raw control register: MXCSR on x86-64, FPCR on arm64.
std::uint64_t fenv_read();

/// Whether a control register holds the default: round to nearest, no flushing, no default NaN.
/// On x86-64 the six sticky exception flags are ignored. Implements RES-05, see A3.4.
bool fenv_is_default(std::uint64_t value);

/// Sets the default environment on this thread. Implements RES-05, see A3.4.
void fenv_reset();

/// Stops the run if this thread's environment is not the default. Implements RES-05, see A3.4.
void fenv_assert_default();

}  // namespace kd::num
