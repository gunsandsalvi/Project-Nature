// A model's recipe (A6.1, PRE-46): the catalogue kind `models`, one entry for each thing put together from the kit's
// parts, read from the art source's models/ as the art lane writes it: art/models/hide_tent_cone/record.toml is the
// entry art:hide_tent_cone. A recipe names the family of Blender parts it uses, what each role of those parts wears,
// and where each part goes: at the thing's origin, round a ring, spanning between a ring and a point on the axis,
// or plugged into a joint of a part already placed. Size, count, angle and material vary by seed, which the assembler
// (view/src/kit_assemble.hpp) works out; a new thing is one more entry (PRN-14). Every field affects only the look, so
// a recipe's change changes its source's look digest and nothing else (PLT-09). The kit's checks (kd_kit check) hold
// each recipe to the parts and joints of its family in the cloud.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "kd/data/catalogue.hpp"
#include "kd/data/schema.hpp"

namespace kd::look {

/// What one role of a model's parts wears: the textures it may be made of, one picked by the seed for the whole thing.
struct ModelMaterial {
    std::string role;
    std::vector<data::Ref> textures;

    template <typename V, typename Self>
    static void visit(V& v, Self& s) {
        using data::Affects;
        v.text({"role", "the material slot's name on the parts: wood, bark, hide, stone, leaf, grass, skin or hair",
                Affects::look},
               s.role);
        v.links(
            {"textures",
             "the textures this role may wear, written art:name; the first is the one a thing without a seed wears, "
             "and a seed picks among them for the whole thing",
             Affects::look},
            s.textures, "textures");
    }
};

/// One placement of a model: a part, or one of several parts, one or more times.
struct ModelPlace {
    std::string name;
    std::vector<std::string> parts;
    std::string rule;
    std::int64_t count = 1;
    std::string joint;
    std::string to_joint;
    std::string onto;
    std::int64_t radius = 0;
    std::int64_t base = 0;
    std::int64_t height = 0;
    std::int64_t turn = 0;
    std::int64_t tilt = 0;
    std::int64_t face = 0;
    std::int64_t jitter_radius = 0;
    std::int64_t jitter_turn = 0;
    std::int64_t jitter_size = 0;

    template <typename V, typename Self>
    static void visit(V& v, Self& s) {
        using data::Affects;
        using data::Measure;
        v.text({"name", "this placement's name, which later placements may plug into", Affects::look}, s.name);
        v.names({"parts", "the parts it may be, by their names in the family; the seed picks one for each copy",
                 Affects::look},
                s.parts);
        v.choice({"rule",
                  "where it goes: root, at the thing's origin; ring, evenly round a circle; span, from the circle to a "
                  "point on the axis; plug, into a joint of an earlier placement",
                  Affects::look},
                 s.rule, {"root", "ring", "span", "plug"});
        v.whole({"count", "how many copies, for a ring or a span; 1 if not written", Affects::look, false}, s.count,
                {1, 512});
        v.text({"joint",
                "the part's joint that goes to the place (its origin if not written); for a span, the one that lies "
                "on the circle",
                Affects::look, false},
               s.joint);
        v.text({"to_joint", "for a span, the part's joint that lies on the axis, where the copies meet", Affects::look,
                false},
               s.to_joint);
        v.text(
            {"onto", "for a plug, the earlier placement and its joint, written placement.joint", Affects::look, false},
            s.onto);
        v.quantity({"radius", "for a ring or a span, the circle's radius", Affects::look, false}, s.radius,
                   Measure::length, {0, 100'000});
        v.quantity({"base", "for a ring or a span, how high the circle lies", Affects::look, false}, s.base,
                   Measure::length, {-10'000, 100'000});
        v.quantity({"height",
                    "for a span, how high the meeting point lies on the axis; for a root or a plug, how far the part "
                    "is lifted",
                    Affects::look, false},
                   s.height, Measure::length, {-10'000, 100'000});
        v.whole({"turn",
                 "degrees clockwise from north: for a ring or a span the first copy's place, for a root the part's "
                 "turn, for a plug its turn about the joint's axis",
                 Affects::look, false},
                s.turn, {-360, 360});
        v.whole({"tilt", "for a root, degrees the part leans over about its own sideways axis", Affects::look, false},
                s.tilt, {-360, 360});
        v.whole({"face", "for a ring, degrees the part turns from facing outward; for a span, its roll about its axis",
                 Affects::look, false},
                s.face, {-360, 360});
        v.quantity({"jitter_radius", "how far each copy's radius may stray, as a share of it, either way",
                    Affects::look, false},
                   s.jitter_radius, Measure::ratio, {0, 500'000});
        v.whole({"jitter_turn", "how far each copy's place round the circle may stray, in degrees either way",
                 Affects::look, false},
                s.jitter_turn, {0, 180});
        v.quantity({"jitter_size", "how much thicker or thinner each copy may be, as a share, either way",
                    Affects::look, false},
                   s.jitter_size, Measure::ratio, {0, 500'000});
    }
};

/// Implements PRE-46 and PRE-42, see A6.1: a model's recipe.
struct Model {
    std::string about;
    std::string family;
    std::string truth;
    std::string approved;
    std::vector<ModelMaterial> materials;
    std::vector<ModelPlace> places;

    template <typename V, typename Self>
    static void visit(V& v, Self& s) {
        using data::Affects;
        v.text({"about", "what it is, in a few words", Affects::look}, s.about);
        v.text({"family", "the family of parts it is made from: art/models/<family>.blend", Affects::look}, s.family);
        v.text({"truth", "its truth check: who looked, when, and what they found (A5.6)", Affects::look}, s.truth);
        v.text({"approved",
                "your words and the date; \"waiting\" while it is offered for your yes or no; or \"not offered yet: \" "
                "and why, while it is below the artwork's level",
                Affects::look},
               s.approved);
        v.records({"material", "what each role of its parts wears, each a table", Affects::look}, s.materials);
        v.records({"place", "where each part goes, in the order they are placed, each a table", Affects::look},
                  s.places);
    }
};

/// Implements MAT-17 for the models: each placement is named once; a ring or a span names its radius, a span its
/// two joints and its height, and a plug an earlier placement and joint; each role is given once and some texture to
/// wear. What needs the parts themselves (that the parts and joints exist, that parts meet at their joints) is the
/// kit's own check (kd_kit check).
void check_models(const data::Catalogue& cat, std::vector<data::Problem>& problems);

}  // namespace kd::look
