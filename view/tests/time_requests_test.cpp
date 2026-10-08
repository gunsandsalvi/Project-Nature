#include "time_requests.hpp"
#include <cmath>
#include <string_view>
#include "doctest.h"
using namespace kd::view;
namespace {
void configured(TimeRequests& time) {
    CHECK(time.configure({{64, 1}, {16, 60}, {2, 480}, {1.0 / 16, 21600}, {1.0 / 128, 259200}}, 1.0 / 4096));
    time.capacity(1e6);
}
}  // namespace
// checks: TIM-01 TIM-04 (T2.9a.3): accepted stops and intermediate zoom rates are continuous and monotonic.
TEST_CASE("zoom time requests match catalogue anchors and interpolate smoothly toward sustainable top speed") {
    TimeRequests time;
    configured(time);
    for (const auto& anchor :
         std::vector<TimeAnchor>{{64, 1}, {16, 60}, {2, 480}, {1.0 / 16, 21600}, {1.0 / 128, 259200}}) {
        time.zoom(anchor.density);
        CHECK(time.resolve().rate == doctest::Approx(anchor.rate));
    }
    double before = 1;
    for (int i = 0; i <= 180; ++i) {
        time.zoom(std::exp2(6.0 - static_cast<double>(i) / 10.0));
        CHECK(time.resolve().rate >= before);
        CHECK(time.resolve().rate <= 1e6);
        before = time.resolve().rate;
    }
    time.zoom(1.0 / 4096);
    CHECK(time.resolve().rate == doctest::Approx(1e6));
}
// checks: TIM-15 TIM-10 (T2.9a.3): pause, skip, manual/lock and director retain their stated priority.
TEST_CASE("time request priority preserves dial and lock through zoom until explicit return to zoom") {
    TimeRequests time;
    configured(time);
    time.zoom(2);
    time.director(120);
    CHECK(std::string_view(time.resolve().source) == "director");
    time.manual(30);
    CHECK(time.resolve().rate == 30);
    CHECK(std::string_view(time.resolve().source) == "manual");
    time.lock(true);
    time.zoom(64);
    CHECK(time.resolve().rate == 30);
    time.skip(900);
    CHECK(time.resolve().rate == 900);
    CHECK(std::string_view(time.resolve().source) == "skip");
    time.pause(true);
    CHECK(time.resolve().rate == 0);
    CHECK(std::string_view(time.resolve().source) == "pause");
    time.pause(false);
    time.skip(std::nullopt);
    CHECK(time.resolve().rate == 30);
    time.clear_manual();
    time.director(std::nullopt);
    CHECK_FALSE(time.locked());
    CHECK(time.resolve().rate == 1);
    time.zoom(2);
    time.lock(true);
    time.zoom(64);
    CHECK(time.resolve().rate == 480);
    time.lock(false);
    CHECK(time.resolve().rate == 1);
}
