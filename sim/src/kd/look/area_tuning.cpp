#include "kd/look/area_tuning.hpp"

#include <array>
#include <string_view>
#include <utility>

#include "kd/look/light_tuning.hpp"
#include "kd/look/model.hpp"
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
        // the camp stands in the strip: from the river's middle line out past its widest bank, up the bank's run, then
        // back from the bank, and the club beyond the tent
        const std::int64_t reach_of_camp =
            a.width / 2 + a.width / 2 * a.width_wobble / 1'000'000 + a.run + a.camp_back + a.club_away;
        if (reach_of_camp > a.strip) {
            problems.push_back(areas.at(i, "camp_back", "puts the camp, with its club, beyond the strip"));
        }
        if (a.form_small >= a.form_simple) {
            problems.push_back(
                areas.at(i, "form_small", "must be fewer pixels than form_simple, the larger form switching first"));
        }
        if (!is_colour_text(a.earth_colour)) {
            problems.push_back(areas.at(i, "earth_colour", "must be a colour written #rrggbb in lower case"));
        }
        // with no art loaded the engine runs without textures or recipes, so the surfaces and the camp's things are
        // held to entries only when there are some
        if (cat.kind<Texture>().size() != 0) {
            const std::array<std::pair<std::string_view, const std::string*>, 3> surfaces{
                {{"ground", &a.ground}, {"bed", &a.bed}, {"marks", &a.marks}}};
            for (const auto& [key, name] : surfaces) {
                if (!cat.find("textures", *name)) {
                    problems.push_back(areas.at(i, key, "names no texture \"" + *name + "\" in the catalogue"));
                }
            }
        }
        if (cat.kind<Model>().size() != 0) {
            const std::array<std::pair<std::string_view, const std::string*>, 4> things{
                {{"tent", &a.tent}, {"tent_simple", &a.tent_simple}, {"tent_small", &a.tent_small}, {"club", &a.club}}};
            for (const auto& [key, name] : things) {
                if (!cat.find("models", *name)) {
                    problems.push_back(areas.at(i, key, "names no recipe \"" + *name + "\" in the catalogue"));
                }
            }
        }
    }
}

}  // namespace kd::look
