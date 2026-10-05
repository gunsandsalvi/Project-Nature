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
#include <string>
#include <string_view>
#include <vector>

#include "kd/chance/chance.hpp"
#include "kd/proof/maths_cases.hpp"
#include "kd/run/workers.hpp"

namespace {

using kd::proof::Args;
using kd::proof::MathsFunction;

// MPFR's function for each of ours, by name.
struct Reference {
    std::string_view name;
    int (*one)(mpfr_ptr, mpfr_srcptr, mpfr_rnd_t);
    int (*two)(mpfr_ptr, mpfr_srcptr, mpfr_srcptr, mpfr_rnd_t);
};

const std::array kReferences = {
    Reference{"sqrt", &mpfr_sqrt, nullptr},       Reference{"cbrt", &mpfr_cbrt, nullptr},
    Reference{"exp", &mpfr_exp, nullptr},         Reference{"exp2", &mpfr_exp2, nullptr},
    Reference{"expm1", &mpfr_expm1, nullptr},     Reference{"log", &mpfr_log, nullptr},
    Reference{"log2", &mpfr_log2, nullptr},       Reference{"log1p", &mpfr_log1p, nullptr},
    Reference{"pow", nullptr, &mpfr_pow},         Reference{"tanh", &mpfr_tanh, nullptr},
    Reference{"erf", &mpfr_erf, nullptr},         Reference{"hypot", nullptr, &mpfr_hypot},
    Reference{"sinpi", &mpfr_sinpi, nullptr},     Reference{"cospi", &mpfr_cospi, nullptr},
    Reference{"tanpi", &mpfr_tanpi, nullptr},     Reference{"asinpi", &mpfr_asinpi, nullptr},
    Reference{"acospi", &mpfr_acospi, nullptr},   Reference{"atanpi", &mpfr_atanpi, nullptr},
    Reference{"atan2pi", nullptr, &mpfr_atan2pi},
};

const Reference* reference_of(std::string_view name) {
    for (const Reference& r : kReferences) {
        if (r.name == name) {
            return &r;
        }
    }
    return nullptr;
}

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
                               std::numeric_limits<double>::max(),
                               -std::numeric_limits<double>::max()};

// MPFR's correctly rounded answer, with the exponent range and the subnormals of a double.
double reference(const Reference& f, Args a) {
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
bool asked(const MathsFunction& f, Args a, double answer) {
    return std::isfinite(answer) && !(f.name == "atan2pi" && a[0] == 0.0 && a[1] == 0.0);
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
    for (const MathsFunction& f : kd::proof::maths_functions()) {
        const Reference* ref = reference_of(f.name);
        if (ref == nullptr) {
            std::printf("  %-8s has no MPFR function here\n", std::string(f.name).c_str());
            return 1;
        }
        std::vector<Args> cases;
        for (double x : kEdges) {
            if (f.arity == 1) {
                cases.push_back({x, 0.0});
                continue;
            }
            for (double y : kEdges) {
                cases.push_back({x, y});
            }
        }
        for (const kd::proof::HardCase& h : kd::proof::hard_cases()) {
            if (h.name == f.name) {
                cases.push_back(h.args);
                cases.push_back({-h.args[0], h.args[1]});
            }
        }
        // the stream the phone's maths suite runs, and as far beyond it as asked
        const std::size_t first_random = cases.size();
        const kd::chance::Draws draws = kd::proof::maths_draws(f);
        for (std::size_t i = 0; i < random; ++i) {
            cases.push_back(f.draw(draws, i));
        }

        // Checked in fixed pieces on the workers, each piece's results kept apart and added in order.
        constexpr std::size_t kPiece = 4096;
        std::vector<Result> pieces((cases.size() + kPiece - 1) / kPiece);
        std::vector<char> usable(cases.size(), 0);
        workers.for_each(pieces.size(), [&](std::size_t p) {
            Result& r = pieces[p];
            for (std::size_t i = p * kPiece; i < std::min(cases.size(), (p + 1) * kPiece); ++i) {
                const double expected = reference(*ref, cases[i]);
                if (!asked(f, cases[i], expected)) {
                    continue;
                }
                usable[i] = 1;
                ++r.tried;
                const double got = f.call(cases[i]);
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
            sink = f.call(a);
        }
        static_cast<void>(sink);
        const double ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - start).count();
        std::printf("  %-8s %9zu %9zu %10.1f\n", std::string(f.name).c_str(), all.tried, all.wrong,
                    timed.empty() ? 0.0 : ns / static_cast<double>(timed.size()));
        for (const auto& e : all.examples) {
            std::printf("    %s(%a, %a): MPFR %a, ours %a\n", std::string(f.name).c_str(), e[0], e[1], e[2], e[3]);
        }
        total += all.tried;
        wrong += all.wrong;
    }
    std::printf("Maths against MPFR: %zu answers, %zu not MPFR's\n", total, wrong);
    return wrong == 0 ? 0 : 1;
}
