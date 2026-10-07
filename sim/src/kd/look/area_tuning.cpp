#include "kd/look/area_tuning.hpp"

#include <array>
#include <string_view>
#include <utility>

#include "kd/look/texture.hpp"

namespace kd::look {

namespace {

// A strip holds the river at its widest, its banks' run and this much besides, in millimetres, so the rows that are
// fine as far as the river and its banks can reach end before the strip does.
constexpr std::int64_t kStripMargin = 2'000;

}  // namespace

void check_area(const data::Catalogue& cat, std::vector<data::Problem>& problems) {
    const data::Kind<AreaTuning>& areas = cat.kind<AreaTuning>();
    for (std::uint32_t i = 0; i < areas.size(); ++i) {
        const AreaTuning& a = areas[i];
        // each wobble is at most a whole (900,000 parts in a million), so half the width at most doubles
        if (a.strip < a.width + a.run + kStripMargin) {
            problems.push_back(
                areas.at(i, "strip", "must be at least the width, the run and two metres, to hold the river"));
        }
        if (a.reach <= a.strip) {
            problems.push_back(
                areas.at(i, "reach", "must be more than the strip, so the meadow lies north and south of it"));
        }
        // with no art loaded the engine runs without textures, so the surfaces are held to entries only when there are
        // some
        if (cat.kind<Texture>().size() == 0) {
            continue;
        }
        const std::array<std::pair<std::string_view, const std::string*>, 3> surfaces{
            {{"ground", &a.ground}, {"bed", &a.bed}, {"marks", &a.marks}}};
        for (const auto& [key, name] : surfaces) {
            if (!cat.find("textures", *name)) {
                problems.push_back(areas.at(i, key, "names no texture \"" + *name + "\" in the catalogue"));
            }
        }
    }
}

}  // namespace kd::look
