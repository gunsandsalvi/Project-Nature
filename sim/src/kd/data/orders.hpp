// The real order of things (MAT-05): lists in each source's checks/orders.toml, each naming a kind, one of its fields
// and some of its entries from the least to the most, such as stones from the softest to the hardest. The check holds
// every listed entry to its place, so a value set by hand can never break an order that decides what is possible.
#pragma once

#include <string>
#include <vector>

#include "kd/data/schema.hpp"
#include "kd/data/toml.hpp"

namespace kd::data {

class Catalogue;

/// One order, an [[order]] table of checks/orders.toml.
struct Order {
    std::string kind;
    std::string field;
    std::vector<Ref> least_to_most;
    std::string why;

    template <typename V, typename Self>
    static void visit(V& v, Self& o) {
        v.text({"kind", "the kind's folder, such as marker"}, o.kind);
        v.text({"field", "the field the order is about, such as speed"}, o.field);
        v.links({"least_to_most", "two entries or more of the kind, from the least to the most"}, o.least_to_most, "");
        v.text({"why", "why the order holds, in plain words"}, o.why);
    }
};

/// Implements MAT-05 and MAT-17, see A3.6: every order in the sources' checks/orders.toml held.
void check_orders(const Catalogue& cat, std::vector<Problem>& problems);

}  // namespace kd::data
