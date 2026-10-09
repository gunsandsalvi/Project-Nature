#pragma once
#include "kd/proof/fixture.hpp"
#include "kd/run/workers.hpp"
namespace kd::proof {
// Frozen living-camp fixture, separate from the accepted M1 catalogue.
[[nodiscard]] std::vector<data::SourceFile> camp_files();
[[nodiscard]] std::string camp_life(run::Workers& workers);
}  // namespace kd::proof
