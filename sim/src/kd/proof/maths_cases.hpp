// The maths functions as the proofs try them (A3.4): each with its name and a stream of inputs inside its domain,
// drawn by keyed chance, and CORE-MATH's hard cases. The phone's self-check runs them in the maths suite, and the
// cloud's oracle (sim/tests/oracle.cpp) holds the same functions to MPFR on the same streams.
#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string_view>

#include "kd/chance/chance.hpp"

namespace kd::proof {

/// A function's arguments; the second is 0 for a function of one.
using Args = std::array<double, 2>;

/// A maths function as the proofs try it. Implements RES-05, see A3.4.
struct MathsFunction {
    std::string_view name;
    int arity;
    double (*call)(Args args);
    /// The index-th input of a stream inside the function's domain, from the function's own draws.
    Args (*draw)(const chance::Draws& draws, std::uint64_t index);
};

/// An input CORE-MATH lists as among the hardest to round for a function, with a finite answer.
struct HardCase {
    std::string_view name;
    Args args;
};

/// Every maths function of kd/num/maths.hpp, in a fixed order.
std::span<const MathsFunction> maths_functions();

/// The function of that name; the name must be one of them.
const MathsFunction& maths_function(std::string_view name);

/// The hard cases (sim/thirdparty/core-math/hard-cases.inc), 256 a function.
std::span<const HardCase> hard_cases();

/// The draws of a function's stream: the same keys on the phone and in the oracle.
chance::Draws maths_draws(const MathsFunction& f);

}  // namespace kd::proof
