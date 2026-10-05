// The maths oracle (RES-05, A3.4): each function of kd/num/maths.hpp against MPFR, which computes the correctly
// rounded answer, on its edges, CORE-MATH's hard cases and 200,000 inputs spread across its domain. Every answer must
// have exactly MPFR's bits, the sign of a zero included. It prints the time each function takes a call, and runs only
// in the cloud, where MPFR is; the phone's builds are held to the same answers by the same-bits check.
//
//     kd_oracle [inputs a function]
#include <mpfr.h>

#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <mutex>
#include <string_view>
#include <vector>

#include "kd/num/maths.hpp"
#include "kd/num/mix.hpp"
#include "kd/run/workers.hpp"

namespace {

using Args = std::array<double, 2>;

// A stream of whole numbers, the same on every run: SplitMix64 keyed by the function.
struct Stream {
    std::uint64_t key;
    std::uint64_t n = 0;
    std::uint64_t next() { return kd::num::mix64(key + 0x9E3779B97F4A7C15ULL * ++n); }
};

constexpr double kMax = std::numeric_limits<double>::max();

// A number in [0, 1) from 53 bits.
double unit(std::uint64_t u) {
    return static_cast<double>(u >> 11U) * 0x1p-53;
}

// A number from lo to hi, spread evenly by value.
double by_value(Stream& s, double lo, double hi) {
    return lo + (hi - lo) * unit(s.next());
}

// A number from 0 to hi, spread evenly over its bits, so each power of two, the subnormals' included, is as likely.
double by_bits(Stream& s, double hi) {
    return std::bit_cast<double>(s.next() % (std::bit_cast<std::uint64_t>(hi) + 1));
}

// The same with either sign.
double signed_bits(Stream& s, double hi) {
    const double x = by_bits(s, hi);
    return (s.next() & 1U) != 0 ? -x : x;
}

// Half the inputs by bits and half by value, from -hi to hi (or from 0 when the domain starts there).
double either(Stream& s, double bits_hi, double lo, double hi) {
    return (s.next() & 1U) != 0 ? signed_bits(s, bits_hi) : by_value(s, lo, hi);
}

// pow's inputs: a positive x by its bits, and a y that brings the answer anywhere from the smallest number to the
// largest; or a negative x to a whole y.
Args pow_inputs(Stream& s) {
    if (s.next() % 5 == 0) {
        return {-by_value(s, 0.25, 4.0), static_cast<double>(static_cast<std::int64_t>(s.next() % 121) - 60)};
    }
    const double x = by_bits(s, kMax);
    const double target = by_value(s, -1074.0, 1023.0);
    const double l = x > 0.0 ? kd::num::log2(x) : 0.0;
    return {x, l != 0.0 ? target / l : by_value(s, -100.0, 100.0)};
}

struct Function {
    const char* name;
    double (*ours)(Args);
    int (*one)(mpfr_ptr, mpfr_srcptr, mpfr_rnd_t);
    int (*two)(mpfr_ptr, mpfr_srcptr, mpfr_srcptr, mpfr_rnd_t);
    Args (*draw)(Stream&);
};

const std::array kFunctions = {
    Function{"sqrt", [](Args a) { return kd::num::sqrt(a[0]); }, &mpfr_sqrt, nullptr,
             [](Stream& s) { return Args{(s.next() & 1U) != 0 ? by_bits(s, kMax) : by_value(s, 0.0, 1e6), 0.0}; }},
    Function{"cbrt", [](Args a) { return kd::num::cbrt(a[0]); }, &mpfr_cbrt, nullptr,
             [](Stream& s) { return Args{either(s, kMax, -1e6, 1e6), 0.0}; }},
    Function{"exp", [](Args a) { return kd::num::exp(a[0]); }, &mpfr_exp, nullptr,
             [](Stream& s) { return Args{either(s, 709.7, -746.0, 709.78), 0.0}; }},
    Function{"exp2", [](Args a) { return kd::num::exp2(a[0]); }, &mpfr_exp2, nullptr,
             [](Stream& s) { return Args{either(s, 1023.9, -1076.0, 1023.99), 0.0}; }},
    Function{"expm1", [](Args a) { return kd::num::expm1(a[0]); }, &mpfr_expm1, nullptr,
             [](Stream& s) { return Args{either(s, 709.7, -746.0, 709.78), 0.0}; }},
    Function{"log", [](Args a) { return kd::num::log(a[0]); }, &mpfr_log, nullptr,
             [](Stream& s) { return Args{(s.next() & 1U) != 0 ? by_bits(s, kMax) : by_value(s, 0.0, 100.0), 0.0}; }},
    Function{"log2", [](Args a) { return kd::num::log2(a[0]); }, &mpfr_log2, nullptr,
             [](Stream& s) { return Args{(s.next() & 1U) != 0 ? by_bits(s, kMax) : by_value(s, 0.0, 100.0), 0.0}; }},
    Function{"log1p", [](Args a) { return kd::num::log1p(a[0]); }, &mpfr_log1p, nullptr,
             [](Stream& s) {
                 const double x = (s.next() & 1U) != 0 ? by_bits(s, kMax) : by_value(s, -1.0, 100.0);
                 return Args{(s.next() & 1U) != 0 && x < 1.0 ? -x : x, 0.0};
             }},
    Function{"pow", [](Args a) { return kd::num::pow(a[0], a[1]); }, nullptr, &mpfr_pow, &pow_inputs},
    Function{"tanh", [](Args a) { return kd::num::tanh(a[0]); }, &mpfr_tanh, nullptr,
             [](Stream& s) { return Args{either(s, kMax, -20.0, 20.0), 0.0}; }},
    Function{"erf", [](Args a) { return kd::num::erf(a[0]); }, &mpfr_erf, nullptr,
             [](Stream& s) { return Args{either(s, kMax, -6.0, 6.0), 0.0}; }},
    Function{"hypot", [](Args a) { return kd::num::hypot(a[0], a[1]); }, nullptr, &mpfr_hypot,
             [](Stream& s) { return Args{either(s, kMax, -1e6, 1e6), either(s, kMax, -1e6, 1e6)}; }},
    Function{"sinpi", [](Args a) { return kd::num::sinpi(a[0]); }, &mpfr_sinpi, nullptr,
             [](Stream& s) { return Args{either(s, kMax, -4.0, 4.0), 0.0}; }},
    Function{"cospi", [](Args a) { return kd::num::cospi(a[0]); }, &mpfr_cospi, nullptr,
             [](Stream& s) { return Args{either(s, kMax, -4.0, 4.0), 0.0}; }},
    Function{"tanpi", [](Args a) { return kd::num::tanpi(a[0]); }, &mpfr_tanpi, nullptr,
             [](Stream& s) { return Args{either(s, kMax, -4.0, 4.0), 0.0}; }},
    Function{"asinpi", [](Args a) { return kd::num::asinpi(a[0]); }, &mpfr_asinpi, nullptr,
             [](Stream& s) { return Args{either(s, 1.0, -1.0, 1.0), 0.0}; }},
    Function{"acospi", [](Args a) { return kd::num::acospi(a[0]); }, &mpfr_acospi, nullptr,
             [](Stream& s) { return Args{either(s, 1.0, -1.0, 1.0), 0.0}; }},
    Function{"atanpi", [](Args a) { return kd::num::atanpi(a[0]); }, &mpfr_atanpi, nullptr,
             [](Stream& s) { return Args{either(s, kMax, -100.0, 100.0), 0.0}; }},
    Function{"atan2pi", [](Args a) { return kd::num::atan2pi(a[0], a[1]); }, nullptr, &mpfr_atan2pi,
             [](Stream& s) { return Args{either(s, kMax, -1e3, 1e3), either(s, kMax, -1e3, 1e3)}; }},
};

// The edges every function is tried at, alone or in pairs: zeros, the smallest and largest numbers of each kind,
// and the small whole numbers and halves where functions change their behaviour.
constexpr std::array kEdges = {0.0,
                               -0.0,
                               std::numeric_limits<double>::denorm_min(),
                               -std::numeric_limits<double>::denorm_min(),
                               0x0.fffffffffffffp-1022,
                               std::numeric_limits<double>::min(),
                               -std::numeric_limits<double>::min(),
                               0.5,
                               -0.5,
                               1.0,
                               -1.0,
                               1.5,
                               2.0,
                               -2.0,
                               0x1p52,
                               0x1p53 + 2.0,
                               709.782712893384,
                               -745.1332191019411,
                               1023.0,
                               -1074.0,
                               kMax,
                               -kMax};

struct Hard {
    std::string_view name;
    double x;
    double y;
};

#define KD_HARD(fn, x, y) Hard{#fn, x, y},
const Hard kHard[] = {
#include "hard-cases.inc"
};
#undef KD_HARD

// MPFR's correctly rounded answer, with the exponent range and the subnormals of a double.
double reference(const Function& f, Args a) {
    mpfr_set_emin(-1073);
    mpfr_set_emax(1024);
    mpfr_t x;
    mpfr_t y;
    mpfr_t r;
    mpfr_inits2(53, x, y, r, static_cast<mpfr_ptr>(nullptr));
    mpfr_set_d(x, a[0], MPFR_RNDN);
    mpfr_set_d(y, a[1], MPFR_RNDN);
    const int inexact = f.one != nullptr ? f.one(r, x, MPFR_RNDN) : f.two(r, x, y, MPFR_RNDN);
    mpfr_subnormalize(r, inexact, MPFR_RNDN);
    const double answer = mpfr_get_d(r, MPFR_RNDN);
    mpfr_clears(x, y, r, static_cast<mpfr_ptr>(nullptr));
    return answer;
}

// Whether the simulation may ask for this at all: an input whose answer is a finite number, and a vector that is not
// zero for atan2pi; kd/num refuses everything else, and its tests check that.
bool asked(const Function& f, Args a, double answer) {
    return std::isfinite(answer) && !(f.two == &mpfr_atan2pi && a[0] == 0.0 && a[1] == 0.0);
}

struct Result {
    std::size_t tried = 0;
    std::size_t wrong = 0;
    std::vector<std::array<double, 4>> examples;  // x, y, MPFR's answer, ours
};

}  // namespace

int main(int argc, char** argv) {
    const std::size_t random = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 200'000;
    kd::run::Workers workers(mpfr_buildopt_tls_p() != 0 ? 4 : 1, "kd-oracle");
    std::printf("Maths against MPFR %s, %zu random inputs a function:\n", mpfr_get_version(), random);
    std::printf("  %-8s %9s %9s %10s\n", "function", "inputs", "wrong", "ns a call");
    std::size_t total = 0;
    std::size_t wrong = 0;
    for (std::size_t index = 0; index < kFunctions.size(); ++index) {
        const Function& f = kFunctions[index];
        std::vector<Args> cases;
        for (double x : kEdges) {
            if (f.one != nullptr) {
                cases.push_back({x, 0.0});
                continue;
            }
            for (double y : kEdges) {
                cases.push_back({x, y});
            }
        }
        for (const Hard& h : kHard) {
            if (h.name == f.name) {
                cases.push_back({h.x, h.y});
                cases.push_back({-h.x, h.y});
            }
        }
        const std::size_t first_random = cases.size();
        Stream stream{kd::num::mix64(index + 1)};
        for (std::size_t i = 0; i < random; ++i) {
            cases.push_back(f.draw(stream));
        }

        // Checked in fixed pieces on the workers, each piece's results kept apart and added in order.
        constexpr std::size_t kPiece = 4096;
        std::vector<Result> pieces((cases.size() + kPiece - 1) / kPiece);
        std::vector<char> usable(cases.size(), 0);
        workers.for_each(pieces.size(), [&](std::size_t p) {
            Result& r = pieces[p];
            for (std::size_t i = p * kPiece; i < std::min(cases.size(), (p + 1) * kPiece); ++i) {
                const double expected = reference(f, cases[i]);
                if (!asked(f, cases[i], expected)) {
                    continue;
                }
                usable[i] = 1;
                ++r.tried;
                const double got = f.ours(cases[i]);
                if (std::bit_cast<std::uint64_t>(got) != std::bit_cast<std::uint64_t>(expected)) {
                    ++r.wrong;
                    if (r.examples.size() < 3) {
                        r.examples.push_back({cases[i][0], cases[i][1], expected, got});
                    }
                }
            }
        });
        Result all;
        for (const Result& r : pieces) {
            all.tried += r.tried;
            all.wrong += r.wrong;
            all.examples.insert(all.examples.end(), r.examples.begin(), r.examples.end());
        }

        // The time a call, over the random inputs the simulation may ask for, on one thread.
        std::vector<Args> timed;
        for (std::size_t i = first_random; i < cases.size(); ++i) {
            if (usable[i] != 0) {
                timed.push_back(cases[i]);
            }
        }
        volatile double sink = 0.0;
        const auto start = std::chrono::steady_clock::now();
        for (const Args& a : timed) {
            sink = f.ours(a);
        }
        static_cast<void>(sink);
        const double ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - start).count();
        std::printf("  %-8s %9zu %9zu %10.1f\n", f.name, all.tried, all.wrong,
                    timed.empty() ? 0.0 : ns / static_cast<double>(timed.size()));
        for (const auto& e : all.examples) {
            std::printf("    %s(%a, %a): MPFR %a, ours %a\n", f.name, e[0], e[1], e[2], e[3]);
        }
        total += all.tried;
        wrong += all.wrong;
    }
    std::printf("Maths against MPFR: %zu answers, %zu not MPFR's\n", total, wrong);
    return wrong == 0 ? 0 : 1;
}
