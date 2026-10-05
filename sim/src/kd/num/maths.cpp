#include "kd/num/maths.hpp"

#include <cmath>

#include "kd/core/check.hpp"

// CORE-MATH's functions (sim/thirdparty/core-math), built as C with the same floating-point flags.
extern "C" {
double cr_cbrt(double x);
double cr_exp(double x);
double cr_exp2(double x);
double cr_expm1(double x);
double cr_log(double x);
double cr_log2(double x);
double cr_log1p(double x);
double cr_pow(double x, double y);
double cr_tanh(double x);
double cr_erf(double x);
double cr_hypot(double x, double y);
double cr_sinpi(double x);
double cr_cospi(double x);
double cr_tanpi(double x);
double cr_asinpi(double x);
double cr_acospi(double x);
double cr_atanpi(double x);
double cr_atan2pi(double y, double x);
}

namespace kd::num {

namespace {

// A function's answer, refused with the function's own message if it or its input is not a finite number: every
// input outside a domain gives NaN or an infinity, so this one check keeps both out of the simulation.
double one(double (*f)(double), double x, const char* message) {
    KD_CHECK(std::isfinite(x), message);
    const double y = f(x);
    KD_CHECK(std::isfinite(y), message);
    return y;
}

double two(double (*f)(double, double), double a, double b, const char* message) {
    KD_CHECK(std::isfinite(a) && std::isfinite(b), message);
    const double y = f(a, b);
    KD_CHECK(std::isfinite(y), message);
    return y;
}

double root(double x) {
    return std::sqrt(x);
}

}  // namespace

double sqrt(double x) {
    return one(&root, x, "num::sqrt needs a finite number of at least 0");
}

double cbrt(double x) {
    return one(&cr_cbrt, x, "num::cbrt needs a finite number");
}

double exp(double x) {
    return one(&cr_exp, x, "num::exp needs a finite x of at most about 709.78");
}

double exp2(double x) {
    return one(&cr_exp2, x, "num::exp2 needs a finite x below 1024");
}

double expm1(double x) {
    return one(&cr_expm1, x, "num::expm1 needs a finite x of at most about 709.78");
}

double log(double x) {
    return one(&cr_log, x, "num::log needs a finite number above 0");
}

double log2(double x) {
    return one(&cr_log2, x, "num::log2 needs a finite number above 0");
}

double log1p(double x) {
    return one(&cr_log1p, x, "num::log1p needs a finite number above -1");
}

double pow(double x, double y) {
    return two(&cr_pow, x, y, "num::pow needs a finite answer: a negative x only to a whole y, 0 not to a y below 0");
}

double tanh(double x) {
    return one(&cr_tanh, x, "num::tanh needs a finite number");
}

double erf(double x) {
    return one(&cr_erf, x, "num::erf needs a finite number");
}

double hypot(double x, double y) {
    return two(&cr_hypot, x, y, "num::hypot needs finite numbers whose length is finite");
}

double sinpi(double x) {
    return one(&cr_sinpi, x, "num::sinpi needs a finite number");
}

double cospi(double x) {
    return one(&cr_cospi, x, "num::cospi needs a finite number");
}

double tanpi(double x) {
    return one(&cr_tanpi, x, "num::tanpi needs a finite x that is not a whole number and a half");
}

double asinpi(double x) {
    return one(&cr_asinpi, x, "num::asinpi needs a number from -1 to 1");
}

double acospi(double x) {
    return one(&cr_acospi, x, "num::acospi needs a number from -1 to 1");
}

double atanpi(double x) {
    return one(&cr_atanpi, x, "num::atanpi needs a finite number");
}

double atan2pi(double y, double x) {
    KD_CHECK(x != 0.0 || y != 0.0, "num::atan2pi needs a vector that is not zero");
    return two(&cr_atan2pi, y, x, "num::atan2pi needs finite numbers");
}

}  // namespace kd::num
