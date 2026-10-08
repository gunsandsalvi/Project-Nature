// T2.9a.2: look-only stream budgets, owned by the catalogue (PLT-04, A18.1).
#pragma once
#include <cstdint>
#include <vector>
#include "kd/data/catalogue.hpp"
#include "kd/data/schema.hpp"
namespace kd::look {
struct StreamTuning {
    std::int64_t queued_jobs = 0;
    std::int64_t preparing_jobs = 0;
    std::int64_t input_bytes = 0;
    std::int64_t prepared_bytes = 0;
    std::int64_t staging_bytes = 0;
    std::int64_t resident_bytes = 0;
    std::int64_t target_bytes = 0;
    std::int64_t maps_bytes = 0;
    std::int64_t sprites_bytes = 0;
    std::int64_t ground_bytes = 0;
    std::int64_t masks_bytes = 0;
    template <typename V, typename Self>
    static void visit(V& v, Self& s) {
        using data::Affects;
        v.whole({"queued_jobs", "bounded pending preparation jobs", Affects::look}, s.queued_jobs, {1, 4096});
        v.whole({"preparing_jobs", "exclusive preparation workers", Affects::look}, s.preparing_jobs, {1, 16});
        const data::Range bytes{1, 8LL * 1024 * 1024 * 1024};
        v.whole({"input_bytes", "owned encoded inputs and metadata", Affects::look}, s.input_bytes, bytes);
        v.whole({"prepared_bytes", "owned CPU images including conversion overlap", Affects::look}, s.prepared_bytes,
                bytes);
        v.whole({"staging_bytes", "upload staging payload, separate from residency", Affects::look}, s.staging_bytes,
                bytes);
        v.whole({"resident_bytes", "aggregate sampled texture allocations", Affects::look}, s.resident_bytes, bytes);
        v.whole({"target_bytes", "render targets allocated separately", Affects::look}, s.target_bytes, bytes);
        v.whole({"maps_bytes", "resident partitioned maps", Affects::look}, s.maps_bytes, bytes);
        v.whole({"sprites_bytes", "resident sprite and material pages", Affects::look}, s.sprites_bytes, bytes);
        v.whole({"ground_bytes", "resident generated ground", Affects::look}, s.ground_bytes, bytes);
        v.whole({"masks_bytes", "resident masks", Affects::look}, s.masks_bytes, bytes);
    }
};
/// Implements PLT-04: partition caps fit the aggregate cap and preparation count fits the queue.
void check_stream_tuning(const data::Catalogue& cat, std::vector<data::Problem>& problems);
}  // namespace kd::look
