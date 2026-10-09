#include "kd/proof/proof.hpp"
#include "kd/proof/camp_cases.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <vector>

#include "kd/chance/chance.hpp"
#include "kd/data/units.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/look/blind.hpp"
#include "kd/look/colour.hpp"
#include "kd/look/frame.hpp"
#include "kd/look/measures.hpp"
#include "kd/num/angle.hpp"
#include "kd/num/convert.hpp"
#include "kd/num/digest.hpp"
#include "kd/num/probability.hpp"
#include "kd/num/torus.hpp"
#include "kd/proof/fixture.hpp"
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

/// A meadow-like picture made by keyed chance: greens, with a yellow flower in about one pixel of 23.
look::Picture made_picture(std::uint64_t seed, std::int64_t width, std::int64_t height) {
    const chance::Draws draws(seed, chance::name("proof"), 0, 0, chance::name("picture"));
    look::Picture p{width, height, {}};
    for (std::int64_t i = 0; i < width * height; ++i) {
        const std::uint64_t b = draws.bits(static_cast<std::uint64_t>(i));
        const bool flower = b % 23 == 0;
        p.rgba.push_back(static_cast<std::uint8_t>(flower ? 235 : 70 + (b >> 8U) % 60));
        p.rgba.push_back(static_cast<std::uint8_t>(flower ? 205 : 100 + (b >> 16U) % 50));
        p.rgba.push_back(static_cast<std::uint8_t>(flower ? 70 : 30 + (b >> 24U) % 40));
        p.rgba.push_back(255);
    }
    return p;
}

/// The look suite: the look's measures (A4.8, A5.5) on pictures made by keyed chance, which the phone must give
/// with the cloud's bits: the colour measures, the target card, people's salience from an object picture, the
/// shimmer after following a projective motion, a dark gradient's levels and the blind test's pairs and code. FLIP,
/// in floats with the platform's maths, is left out. Implements PRE-01, PRE-20, PRE-22 and PRE-28, see A4.8.
std::string look_measures(run::Workers& /*workers*/) {
    constexpr std::int64_t kSide = 192;
    num::Digest digest;
    const look::Picture a = made_picture(1, kSide, kSide);
    const look::Picture b = made_picture(2, kSide, kSide);
    const look::Stats s = look::stats(a);
    for (const double v :
         {s.lightness, s.colourfulness, s.hue, s.contrast, s.texel_contrast, s.accents.value_or(-1.0)}) {
        digest.f64(v);
    }
    const look::Card c = look::card(a);
    for (const double v : {c.lightness, c.dark, c.lights_hue, c.lights, c.shade, c.strong_colour, c.green,
                           c.green_chroma, c.flat, c.things, c.largest_colour, c.texture, c.masses}) {
        digest.f64(v);
    }
    // three people in the engine's number colours, 4 by 4 pixels each, and one more out of the frame
    look::Picture objects{kSide, kSide, std::vector<std::uint8_t>(static_cast<std::size_t>(kSide * kSide * 4), 16)};
    for (std::int64_t person = 1; person <= 3; ++person) {
        for (std::int64_t y = 40 * person; y < 40 * person + 4; ++y) {
            for (std::int64_t x = 50 * person; x < 50 * person + 4; ++x) {
                objects.rgba[static_cast<std::size_t>((y * kSide + x) * 4)] =
                    static_cast<std::uint8_t>(32 * person + 16);
            }
        }
    }
    const look::Salience salience = look::salience(a, look::numbers(objects), {1, 2, 3, 4});
    for (const double v : salience.percentiles) {
        digest.f64(v);
    }
    digest.f64(salience.median);
    digest.f64(salience.least);
    const std::vector<std::optional<double>> followed =
        look::follow(look::error(a, b), kSide, kSide, {1.0, 0.02, 1.5, 0.0, 1.0, -0.5, 0.0, 0.0001, 1.0});
    digest.f64(look::flicker(followed, look::error(b, a), 3.0));
    // a dark gradient, as a moonlit slope's levels
    look::Picture slope{256, 4, {}};
    for (std::int64_t y = 0; y < 4; ++y) {
        for (std::int64_t x = 0; x < 256; ++x) {
            const auto v = static_cast<std::uint8_t>(10 + x / 16);
            slope.rgba.insert(slope.rgba.end(), {v, v, static_cast<std::uint8_t>(v + 4), 255});
        }
    }
    const look::Levels levels = look::levels(slope);
    digest.i64(levels.distinct);
    digest.i64(levels.widest);
    for (const look::BlindPair& p : look::blind_pairs(look::Comparison::msaa, 54'321)) {
        digest.u8(p.better_first ? 1 : 0);
        digest.f64(p.heading);
        digest.i64(p.east);
        digest.i64(p.north);
    }
    digest.text(look::blind_code(
        {look::Comparison::msaa, 54'321, {true, false, true, true, false, false, true, false, true, true}}));
    return digest.hex();
}

/// A crowd of 1,000 markers in 40 camps, from the fixed fixture catalogue, run for some game days, each day's digest
/// of the whole state taken at midnight: one event at a time, or in islands on the workers.
std::string crowd_days(run::Workers* workers, time::Seconds days) {
    data::Catalogue catalogue;
    const std::vector<data::SourceFile> files = fixture_files();
    KD_CHECK(catalogue.load(files).empty(), "proof: the fixture catalogue has problems");
    demo::CrowdWorld crowd(20'260'105, catalogue, 40);
    num::Digest digest;
    for (time::Seconds day = 1; day <= days; ++day) {
        if (workers != nullptr) {
            crowd.world().run_islands(day * time::kDay, *workers, 300);
        } else {
            crowd.world().run_to(day * time::kDay);
        }
        digest.u64(crowd.world().digests().whole);
    }
    digest.u64(crowd.world().events_run());
    digest.u64(crowd.world().history_count());
    return digest.hex();
}

/// The world suite: the crowd run one event at a time for 30 game days. Implements RES-05 and TIM-17, see A3.3.
std::string world(run::Workers& /*workers*/) {
    return crowd_days(nullptr, 30);
}

/// The islands suite: the same crowd for 3 game days in islands of 5-minute windows on the workers, which must give
/// the one-event-at-a-time run's digest, checked here, on any number of threads. Implements RES-05 and WLD-13, see
/// A3.3.
std::string islands(run::Workers& workers) {
    const std::string in_islands = crowd_days(&workers, 3);
    KD_CHECK(in_islands == crowd_days(nullptr, 3), "proof: the islands gave another world than one event at a time");
    return in_islands;
}

constexpr std::array kSuites = {
    Suite{"camp_dreams", "queued and delivered place dreams, nightly caps and saved influence", &camp_dreams},
    Suite{"camp_life", "saved needs, actions, memories and finite renewal on one/four workers", &camp_life},
    Suite{"smoke", "arithmetic, square roots and a sum in fixed pieces", &smoke},
    Suite{"maths", "every maths function on its stream of inputs and its hard cases", &maths},
    Suite{"chance", "a million keyed draws of every kind", &chance_draws},
    Suite{"torus", "ways, distances and directions between places on the world", &torus},
    Suite{"units", "quantities and chances written as text, read exactly", &units},
    Suite{"world", "a crowd of 1,000 markers walking, greeting and sleeping for 30 game days, one event at a time",
          &world},
    Suite{"islands", "the crowd for 3 game days in islands on the workers, the same as one event at a time", &islands},
    Suite{"look", "the look's measures on pictures made by chance, FLIP aside, and the blind test's pairs",
          &look_measures},
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
