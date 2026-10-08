#include "kd/look/stream_tuning.hpp"
namespace kd::look {
void check_stream_tuning(const data::Catalogue& cat, std::vector<data::Problem>& problems) {
    const auto& entries = cat.kind<StreamTuning>();
    for (std::uint32_t i = 0; i < entries.size(); ++i) {
        const auto& s = entries[i];
        if (s.maps_bytes + s.sprites_bytes + s.ground_bytes + s.masks_bytes > s.resident_bytes) {
            problems.push_back(
                entries.at(i, "resident_bytes", "resident partition caps exceed the aggregate sampled texture cap"));
        }
        if (s.preparing_jobs > s.queued_jobs) {
            problems.push_back(entries.at(i, "preparing_jobs", "preparation count exceeds the bounded pending queue"));
        }
    }
}
}  // namespace kd::look
