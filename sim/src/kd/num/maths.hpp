// The simulation's maths functions (A3.4): CORE-MATH's, which round correctly, so each answer is unique and every
// machine gives the same bits, and the square root, which IEEE 754 already rounds correctly. Nothing else in sim/
// may call the platform's maths, which differs between the cloud and the phone.
//
// Each function refuses an input outside its domain and an answer that is not a finite number, so no NaN or infinity
// ever enters the simulation. Angles are turns (kd/num/angle.hpp), so the trigonometric functions are those in half
// turns: sinpi(x) is sin(pi x), and asinpi(x) is asin(x) / pi.
#pragma once

namespace kd::num {

/// The square root of a number of at least 0. Implements RES-05, see A3.4.
double sqrt(double x);
/// The cube root. Implements RES-05, see A3.4.
double cbrt(double x);
/// e to the x, which must not pass the largest number. Implements RES-05, see A3.4.
double exp(double x);
/// 2 to the x. Implements RES-05, see A3.4.
double exp2(double x);
/// e to the x, less 1, exact for small x. Implements RES-05, see A3.4.
double expm1(double x);
/// The natural logarithm of a number above 0. Implements RES-05, see A3.4.
double log(double x);
/// The logarithm in base 2 of a number above 0. Implements RES-05, see A3.4.
double log2(double x);
/// The natural logarithm of 1 + x, for x above -1, exact for small x. Implements RES-05, see A3.4.
double log1p(double x);
/// x to the power y: a negative x only to a whole y, and 0 only to a y of at least 0. Implements RES-05, see A3.4.
double pow(double x, double y);
/// The hyperbolic tangent. Implements RES-05, see A3.4.
double tanh(double x);
/// The error function. Implements RES-05, see A3.4.
double erf(double x);
/// The length of (x, y), without overflow on the way. Implements RES-05, see A3.4.
double hypot(double x, double y);
/// sin(pi x): the sine of x half turns. Implements RES-05, see A3.4.
double sinpi(double x);
/// cos(pi x). Implements RES-05, see A3.4.
double cospi(double x);
/// tan(pi x), except at odd quarter turns, where it has no value. Implements RES-05, see A3.4.
double tanpi(double x);
/// asin(x) / pi, in half turns, for x from -1 to 1. Implements RES-05, see A3.4.
double asinpi(double x);
/// acos(x) / pi, in half turns, for x from -1 to 1. Implements RES-05, see A3.4.
double acospi(double x);
/// atan(x) / pi, in half turns. Implements RES-05, see A3.4.
double atanpi(double x);
/// The direction of (x, y) in half turns from the x axis, from -1 to 1; the vector must not be zero, whose direction
/// would depend on the signs of its zeros. Implements RES-05, see A3.4.
double atan2pi(double y, double x);

}  // namespace kd::num
