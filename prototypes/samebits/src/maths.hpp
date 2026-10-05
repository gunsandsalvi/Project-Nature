// P5 The same bits (IMPLEMENTATION α0.4a, A3.4): our own sine, cosine, exponent, logarithm and power, built from
// IEEE adds, multiplies and divides only, so they give the same bits on every chip that rounds as IEEE says. The
// platform's versions may differ in their last bit between libraries and chips. The square root, which IEEE requires
// to be correctly rounded, is the platform's. Pre-production code (research 00).
#pragma once

namespace samebits {

double sine(double x);
double cosine(double x);
double exponent(double x);
// The natural logarithm of x > 0.
double logarithm(double x);
// a to the power b, for a > 0.
double power(double a, double b);

}  // namespace samebits
