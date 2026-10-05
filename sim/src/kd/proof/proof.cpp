#include "kd/proof/proof.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

#include "kd/num/digest.hpp"

namespace kd::proof {

namespace {

/// The smoke suite: plain arithmetic, square roots and a sum cut into fixed pieces, which every build must agree on
/// before anything else is worth comparing. Implements RES-05, see A3.4.
std::string smoke(run::Workers& workers) {
    num::Digest digest;

    // A chain of dependent operations, where any difference in rounding or fusing grows into a different number.
    double x = 0.5;
    for (int k = 0; k < 200'000; ++k) {
        x = std::sqrt(x * x + 1.25) / (1.0 + x * 0.375) + static_cast<double>(k % 7) * 0.001;
        if (k % 1000 == 0) {
            digest.f64(x);
        }
    }

    // A sum of a million terms cut into 256 fixed pieces, each piece summed by whichever thread takes it and the
    // pieces added in their order, so the total cannot depend on the number of threads.
    constexpr std::size_t kTerms = std::size_t{1} << 20;
    constexpr std::size_t kPiece = 4096;
    std::vector<double> pieces(kTerms / kPiece, 0.0);
    workers.for_each(pieces.size(), [&](std::size_t p) {
        double sum = 0.0;
        for (std::size_t i = p * kPiece; i < (p + 1) * kPiece; ++i) {
            const double term = std::sqrt(static_cast<double>(i) + 0.5) / (1.0 + static_cast<double>(i % 97));
            sum += (i % 2 == 0) ? term : -term;
        }
        pieces[p] = sum;
    });
    double total = 0.0;
    for (double piece : pieces) {
        digest.f64(piece);
        total += piece;
    }
    digest.f64(total);

    // Whole-number mixing, which no build may change at all.
    std::uint64_t h = 0x9E3779B97F4A7C15ULL;
    for (std::uint64_t i = 0; i < 100'000; ++i) {
        h ^= i;
        h *= 0xBF58476D1CE4E5B9ULL;
        h ^= h >> 31;
    }
    digest.u64(h);
    return digest.hex();
}

constexpr std::array kSuites = {
    Suite{"smoke", "arithmetic, square roots and a sum in fixed pieces", &smoke},
};

}  // namespace

std::span<const Suite> suites() {
    return kSuites;
}

std::string run(std::string_view name, run::Workers& workers) {
    for (const Suite& s : kSuites) {
        if (s.name == name) {
            return s.run(workers);
        }
    }
    return {};
}

}  // namespace kd::proof
