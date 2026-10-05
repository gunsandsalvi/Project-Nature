#include "kd/demo/checks.hpp"

#include "kd/data/catalogue.hpp"
#include "kd/demo/marker.hpp"

namespace kd::demo {

void check_company(const data::Catalogue& cat, std::vector<data::Problem>& problems) {
    const data::Kind<Marker>& markers = cat.kind<Marker>();
    std::vector<std::uint8_t> named(markers.size(), 0);
    for (std::uint32_t i = 0; i < markers.size(); ++i) {
        for (const data::Ref& r : markers[i].walks_with) {
            named[r.index] = 1;
        }
    }
    for (std::uint32_t i = 0; i < markers.size(); ++i) {
        if (markers[i].walks_with.empty() && named[i] == 0) {
            problems.push_back(markers.at(i, "walks_with",
                                          markers.name(i) +
                                              " keeps company with no one: it names no marker in walks_with and no "
                                              "marker names it, so it would walk the crowd alone"));
        }
    }
}

}  // namespace kd::demo
