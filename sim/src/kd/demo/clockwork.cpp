#include "kd/demo/clockwork.hpp"

#include <algorithm>

#include "kd/core/check.hpp"
#include "kd/num/mix.hpp"
#include "kd/num/whole.hpp"

namespace kd::demo {

Clockwork::Clockwork(std::uint64_t work_per_hour) : work_(work_per_hour) {
    KD_CHECK(work_per_hour >= 1, "demo::Clockwork needs some work for each hour");
}

time::Seconds Clockwork::advance(time::Seconds frontier, time::Seconds goal) {
    const time::Seconds end = std::min(goal, frontier + time::kDay);
    // every hour that begins after the frontier and no later than the batch's end
    for (time::Seconds hour = (num::floor_div(frontier, time::kHour) + 1) * time::kHour; hour <= end;
         hour += time::kHour) {
        std::uint64_t z = state_ ^ static_cast<std::uint64_t>(hour);
        for (std::uint64_t i = 0; i < work_; ++i) {
            z = num::mix64(z + i);
        }
        state_ = z;
    }
    return end;
}

}  // namespace kd::demo
