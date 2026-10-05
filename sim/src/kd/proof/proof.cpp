#include "kd/proof/proof.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

#include "kd/chance/chance.hpp"
#include "kd/data/units.hpp"
#include "kd/num/angle.hpp"
#include "kd/num/convert.hpp"
#include "kd/num/digest.hpp"
#include "kd/num/probability.hpp"
#include "kd/num/torus.hpp"
#include "kd/proof/maths_cases.hpp"

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

/// The maths suite: every maths function on 20,000 inputs of its stream, cut into fixed pieces on the workers, and
/// on CORE-MATH's hard cases; each answer's bits digested in order. Implements RES-05, see A3.4.
std::string maths(run::Workers& workers) {
    constexpr std::uint64_t kInputs = 20'000;
    constexpr std::uint64_t kPiece = 1'000;
    num::Digest digest;
    for (const MathsFunction& f : maths_functions()) {
        const chance::Draws draws = maths_draws(f);
        std::vector<double> answers(kInputs);
        workers.for_each(kInputs / kPiece, [&](std::size_t p) {
            for (std::uint64_t i = p * kPiece; i < (p + 1) * kPiece; ++i) {
                answers[i] = f.call(f.draw(draws, i));
            }
        });
        digest.text(f.name);
        for (double a : answers) {
            digest.f64(a);
        }
    }
    for (const HardCase& h : hard_cases()) {
        digest.f64(maths_function(h.name).call(h.args));
    }
    return digest.hex();
}

/// The chance suite: 2,000 beings, each drawing 100 times in every way at its own moment, the beings cut into fixed
/// pieces on the workers and each piece's digest gathered in order. Implements TIM-16, see A3.5.
std::string chance_draws(run::Workers& workers) {
    constexpr std::uint64_t kBeings = 2'000;
    constexpr std::uint64_t kPiece = 50;
    constexpr std::uint64_t kDraws = 100;
    const chance::Name system = chance::name("proof");
    const chance::Name purpose = chance::name("chance");
    const num::Probability third = num::Probability::ratio(1, 3);
    std::vector<std::uint64_t> pieces(kBeings / kPiece);
    workers.for_each(pieces.size(), [&](std::size_t p) {
        num::Digest digest;
        for (std::uint64_t being = p * kPiece; being < (p + 1) * kPiece; ++being) {
            const auto moment = static_cast<std::int64_t>(being * 3'600) - 7'200;
            const chance::Draws draws(20261005, system, being, moment, purpose);
            for (std::uint64_t i = 0; i < kDraws; ++i) {
                digest.u64(draws.bits(i));
                digest.u64(draws.below(i, 1'000));
                digest.i64(draws.between(i, -500, 500));
                digest.f64(draws.fraction(i));
                digest.u8(draws.fires(i, third) ? 1 : 0);
            }
        }
        pieces[p] = digest.value();
    });
    num::Digest digest;
    for (std::uint64_t piece : pieces) {
        digest.u64(piece);
    }
    return digest.hex();
}

/// The torus suite: 200,000 pairs of places spread over the world of WLD-03, with the way between them, its
/// distance, the place it leads to, its direction and that direction's sine and cosine. Implements RES-05 and WLD-01,
/// see A3.4.
std::string torus(run::Workers& workers) {
    constexpr std::uint64_t kPairs = 200'000;
    constexpr std::uint64_t kPiece = 2'000;
    const num::Torus world(200'000'000, 100'000'000);
    const chance::Draws draws(20261005, chance::name("proof"), 0, 0, chance::name("torus"));
    std::vector<std::uint64_t> pieces(kPairs / kPiece);
    workers.for_each(pieces.size(), [&](std::size_t p) {
        num::Digest digest;
        for (std::uint64_t i = p * kPiece; i < (p + 1) * kPiece; ++i) {
            const auto w = static_cast<std::uint64_t>(world.width());
            const auto h = static_cast<std::uint64_t>(world.height());
            const num::Point a{static_cast<std::int32_t>(draws.below(4 * i, w)),
                               static_cast<std::int32_t>(draws.below(4 * i + 1, h))};
            const num::Point b{static_cast<std::int32_t>(draws.below(4 * i + 2, w)),
                               static_cast<std::int32_t>(draws.below(4 * i + 3, h))};
            const num::Offset way = world.offset(a, b);
            const std::int64_t distance = world.distance(a, b);
            const num::Point there = world.moved(a, way);
            digest.i64(way.dx);
            digest.i64(way.dy);
            digest.i64(world.squared_distance(a, b));
            digest.i64(distance);
            digest.u32(static_cast<std::uint32_t>(there.x));
            digest.u32(static_cast<std::uint32_t>(there.y));
            if (way.dx != 0 || way.dy != 0) {
                const num::Angle heading = num::direction(way);
                const double east = num::cos(heading);
                digest.u32(heading.steps);
                digest.f64(east);
                digest.f64(num::sin(heading));
                digest.i64(num::to_int(east * static_cast<double>(distance), num::Round::nearest));
            }
        }
        pieces[p] = digest.value();
    });
    num::Digest digest;
    for (std::uint64_t piece : pieces) {
        digest.u64(piece);
    }
    return digest.hex();
}

// A quantity or a chance written as an author might, right or wrong: whole and decimal numbers in every unit of a
// measure, some in two parts, some with a comma or an unknown unit. Each draw is its own statement, so their order
// is fixed (A3.4).
std::string written(const chance::Draws& draws, std::uint64_t i, int& measure) {
    std::uint64_t k = i * 32;
    const auto draw = [&](std::uint64_t n) { return draws.below(k++, n); };
    measure = static_cast<int>(draw(10));
    const auto number = [&] {
        std::string text = std::to_string(draw(100'000));
        const auto places = static_cast<std::size_t>(draw(4));
        if (places > 0) {
            std::string decimals = std::to_string(draw(10'000));
            decimals.insert(0, 4 - decimals.size(), '0');
            const bool comma = draw(30) == 0;
            text += (comma ? "," : ".") + decimals.substr(0, places);
        }
        return text;
    };
    if (measure == 9) {
        const std::uint64_t form = draw(3);
        if (form == 0) {
            return std::to_string(draw(120)) + "%";
        }
        if (form == 1) {
            const std::uint64_t one = draw(5);
            return std::to_string(one) + " in " + std::to_string(draw(1'000));
        }
        return "0." + std::to_string(draw(1'000'000));
    }
    const auto units = data::units_of(static_cast<data::Measure>(measure));
    std::string text = draw(10) == 0 ? "-" : "";
    const std::uint64_t parts = draw(4) == 0 ? 2 : 1;
    for (std::uint64_t part = 0; part < parts; ++part) {
        if (part > 0) {
            text += " ";
        }
        text += number();
        if (draw(2) == 0) {
            text += " ";
        }
        text += units[draw(units.size())].name;
        if (draw(40) == 0) {
            text += "x";
        }
    }
    return text;
}

/// The units suite: 200,000 quantities and chances written as text, read exactly, each value or refusal digested
/// in order: the phone's reading of the catalogues can never differ from the cloud's. Implements MAT-13, see A3.6.
std::string units(run::Workers& workers) {
    constexpr std::uint64_t kStrings = 200'000;
    constexpr std::uint64_t kPiece = 2'000;
    const chance::Draws draws(20261005, chance::name("proof"), 0, 0, chance::name("units"));
    std::vector<std::uint64_t> pieces(kStrings / kPiece);
    workers.for_each(pieces.size(), [&](std::size_t p) {
        num::Digest digest;
        for (std::uint64_t i = p * kPiece; i < (p + 1) * kPiece; ++i) {
            int measure = 0;
            const std::string text = written(draws, i, measure);
            digest.text(text);
            if (measure == 9) {
                const data::Chance c = data::read_probability(text);
                digest.u64(c.value.threshold());
                digest.u8(c.value.certain() ? 1 : 0);
                digest.text(c.error);
            } else {
                const data::Amount a = data::read_quantity(text, static_cast<data::Measure>(measure));
                digest.i64(a.value);
                digest.text(a.error);
            }
        }
        pieces[p] = digest.value();
    });
    num::Digest digest;
    for (std::uint64_t piece : pieces) {
        digest.u64(piece);
    }
    return digest.hex();
}

constexpr std::array kSuites = {
    Suite{"smoke", "arithmetic, square roots and a sum in fixed pieces", &smoke},
    Suite{"maths", "every maths function on its stream of inputs and its hard cases", &maths},
    Suite{"chance", "a million keyed draws of every kind", &chance_draws},
    Suite{"torus", "ways, distances and directions between places on the world", &torus},
    Suite{"units", "quantities and chances written as text, read exactly", &units},
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
