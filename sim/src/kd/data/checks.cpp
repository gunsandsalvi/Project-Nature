#include "kd/data/checks.hpp"

namespace kd::data {

std::vector<Problem> run_checks(const Catalogue& cat) {
    std::vector<Problem> problems;
    for (const Check& c : checks()) {
        c.run(cat, problems);
    }
    return problems;
}

}  // namespace kd::data
