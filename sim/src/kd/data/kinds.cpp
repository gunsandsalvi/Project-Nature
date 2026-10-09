// Every kind of catalogue entry the game knows, one line each, and the checks they bring (A3.6): a new kind is its
// header, its visit(), a line here and its checks' lines below.
#include <array>

#include "kd/data/catalogue.hpp"
#include "kd/data/checks.hpp"
#include "kd/data/craft.hpp"
#include "kd/data/orders.hpp"
#include "kd/demo/checks.hpp"
#include "kd/demo/crowd.hpp"
#include "kd/demo/discovery_scene.hpp"
#include "kd/demo/living_rules.hpp"
#include "kd/demo/marker.hpp"
#include "kd/look/area_tuning.hpp"
#include "kd/look/card.hpp"
#include "kd/look/light_tuning.hpp"
#include "kd/look/model.hpp"
#include "kd/look/navigation.hpp"
#include "kd/look/sprite.hpp"
#include "kd/look/stream_tuning.hpp"
#include "kd/look/texture.hpp"
#include "kd/look/water_tuning.hpp"
#include "kd/run/heat_tuning.hpp"
#include "kd/run/save_tuning.hpp"
#include "kd/time/speeds.hpp"

namespace kd::data {

Catalogue::Catalogue() {
    add_kind<demo::DiscoveryScene>("tuning/discovery", "finite labelled Discovery camp inputs", Layout::single);
    add_kind<ItemKind>("item", "conserved craft materials and forms");
    add_kind<Blueprint>("blueprint", "generic characteristic-constrained work");
    add_kind<demo::LivingRules>("tuning/living", "scoped adult camp rates and bounded renewal", Layout::single);
    add_kind<demo::NeedUse>("need_use", "known direct body need affordances");
    add_kind<look::Moment>("card", "a moment's bands on the target card, from the pictures you chose (PRE-01)");
    add_kind<demo::Marker>("marker", "a kind of the demonstration's markers, which walk, meet and greet (MAT-16)");
    add_kind<look::Model>("models", "a thing put together from the kit's parts: its recipe (PRE-46)", Layout::records);
    add_kind<look::Texture>("textures",
                            "a texture's record: its levels, where it came from, its checks and your OK (PRE-20)",
                            Layout::records);
    add_kind<look::SpriteFamily>("sprites", "an authored family with aligned channels and exact pivots (PRE-22)",
                                 Layout::records);
    add_kind<look::NavigationTuning>("tuning/navigation", "look navigation anchors and release motion (PRE-03)",
                                     Layout::single);
    add_kind<look::StreamTuning>("tuning/stream", "bounded look preparation and graphics allocation budgets (PLT-04)",
                                 Layout::single);
    add_kind<look::AreaTuning>(
        "tuning/area",
        "the stand-in area: the surfaces its meadow, river bed and water wear, and the river's shape (PRE-23, PRE-26)",
        Layout::single);
    add_kind<look::LightTuning>(
        "tuning/light",
        "the look's light: where the sun stands and the colours of its light, the sky and the haze (PRE-30)",
        Layout::single);
    add_kind<look::CardTuning>("tuning/card", "the target card's goals and slack (PRE-01)", Layout::single);
    add_kind<demo::Crowd>("tuning/crowd", "the demonstration's crowd: its camps, markers and greetings (MAT-16)",
                          Layout::single);
    add_kind<run::HeatTuning>("tuning/heat", "how time slows before the phone throttles (PLT-01)", Layout::single);
    add_kind<run::SaveTuning>("tuning/saves", "how often a running world is saved (PLT-07)", Layout::single);
    add_kind<time::ZoomSpeeds>("tuning/time", "the speeds of time at the zoom stops (TIM-01)", Layout::single);
    add_kind<look::WaterTuning>("tuning/water",
                                "the river's water: how it takes the bed's colour, its current and its marks (PRE-26)",
                                Layout::single);
}

namespace {

// In the order of their names.
constexpr std::array<Check, 11> kChecks{{
    {"area", "PRE-26", "the strip holds the river and its banks, and the meadow lies north and south of it",
     look::check_area},
    {"company", "MAT-17", "every demonstration marker can greet or be greeted", demo::check_company},
    {"craft", "MAT-17", "craft roles have valid characteristic bounds and conserved results", check_craft},
    {"light", "PRE-30", "every colour of the look's light is written #rrggbb", look::check_light},
    {"living", "MAT-17", "scoped camp affordances fit their body quantities and durations", demo::check_living},
    {"models", "PRE-46",
     "every model's placements are named once, and a ring, span or plug names what it needs and a role wears something",
     look::check_models},
    {"navigation", "PRE-03", "ordered time and form anchors fit navigation limits", look::check_navigation},
    {"orders", "MAT-05", "every entry in its place in the real order of things, as checks/orders.toml lists them",
     check_orders},
    {"sprites", "PRE-22", "authored family channels, rectangles, pivots and parts align", look::check_sprites},
    {"stream", "PLT-04", "resident partition caps fit the aggregate and preparation fits the queue",
     look::check_stream_tuning},
    {"water", "PRE-26", "every colour of the river's water is written #rrggbb", look::check_water},
}};

}  // namespace

std::span<const Check> checks() {
    return kChecks;
}

}  // namespace kd::data
