// The assembler (A6.1, PRE-46): a recipe's placements worked out into where each part of a thing goes, varied by a
// seed, and the check that a recipe fits its family's parts. It touches no Godot, so its tests run alone; the kit class
// draws what it makes (view/src/kit_draw.hpp).
//
// A thing's own metres are Godot's: x east, y up, z south, its origin on the ground at its middle. A placement is one
// of
//   root  the part at the origin, turned and leaned and lifted;
//   ring  copies evenly round a circle, each facing outward;
//   span  copies from the circle to a point on the axis, each turned and stretched along its own length so that one
//         joint lies on the circle and another at the meeting point, as the poles of a cone tent do;
//   plug  copies into a joint of each copy of an earlier placement, the two joints' frames made one.
// A seed picks, for each copy, which of the placement's parts it is, how far it strays from its place, and how thick it
// is, and, for the whole thing, which texture each role wears.
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "kd/look/model.hpp"
#include "kit.hpp"

namespace kd::view::kit {

/// One part put in its place: the placement it came from, the part and the carrying that takes its own metres to the
/// thing's: a point p goes to the first three rows applied to it, plus the last column, as three rows of four.
struct Placed {
    std::string place;
    std::string part;
    std::array<double, 12> matrix{1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0};
};

/// The thing a recipe makes for a seed.
struct Assembly {
    std::vector<Placed> placed;
    /// For each role the recipe names, the texture it wears for this seed: the role and the texture's name.
    std::vector<std::pair<std::string, std::string>> wears;
    std::array<double, 3> lowest{};
    std::array<double, 3> highest{};
    /// Empty when the thing was put together; otherwise why it could not be.
    std::string problem;
};

/// Implements PRE-46 and PRE-42, see A6.1: the thing a recipe makes from its family's parts for a seed, the same
/// every time for the same seed. A recipe that does not fit its parts gives its problem and nothing placed.
[[nodiscard]] Assembly assemble(const kd::look::Model& model, const Family& family, std::uint64_t seed);

/// A point of a part carried to the thing's metres by a placed part's carrying.
[[nodiscard]] std::array<double, 3> carried(const Placed& placed, const std::array<float, 3>& point);

/// Implements PRE-46, see A6.1 and A6.4: a recipe held to its family: every part and joint it names is there; every
/// role its parts wear has a texture; a span's parts are as long as the span is, to within a seventh, with the
/// radius straying as far as it may; and each plug's joints meet. Each fault in a sentence naming the model.
[[nodiscard]] std::vector<std::string> check_model(const std::string& name, const kd::look::Model& model,
                                                   const Family& family);

}  // namespace kd::view::kit
