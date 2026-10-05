// P6's run from the command line (TIM-07, MND-15): a thousand people for some game days on some threads, timed,
// with the time each part of the minds took and the state's checksum at the end.
//   minds_cli [threads] [days]
// Pre-production code (research 00).
#include <chrono>
#include <cstdio>
#include <cstdlib>

#include "hash.hpp"
#include "minds.hpp"

int main(int argc, char** argv) {
    using Clock = std::chrono::steady_clock;
    minds::Settings settings;
    int days = 10;
    if (argc > 1) {
        settings.threads = std::atoi(argv[1]);
    }
    if (argc > 2) {
        days = std::atoi(argv[2]);
    }
    const auto t0 = Clock::now();
    minds::World world(settings);
    const double made = std::chrono::duration<double>(Clock::now() - t0).count();
    const auto t1 = Clock::now();
    for (int d = 0; d < days; ++d) {
        world.run_day();
    }
    const double seconds = std::chrono::duration<double>(Clock::now() - t1).count();
    const minds::Times& t = world.times();
    const double total = t.land + t.choice + t.paths + t.talk + t.other;
    const minds::TripCounts trips = world.trips();
    std::printf("%zu people on %d threads: %d game days in %.2f s; the world made in %.2f s\n", world.people().size(),
                settings.threads, days, seconds, made);
    // a game year is 60 days, so game years a real minute are game days a real second
    std::printf("game years a real minute: %.2f\n", days / seconds);
    std::printf("decisions: %lld, %.0f a game day, %.0f a second; %lld activities cut short by a need\n",
                static_cast<long long>(world.decisions()), static_cast<double>(world.decisions()) / days,
                static_cast<double>(world.decisions()) / seconds, static_cast<long long>(world.cut_short()));
    std::printf("time by part: results %.0f%%, choice %.0f%%, paths %.0f%%, talk %.0f%%, other %.0f%%\n",
                100.0 * t.land / total, 100.0 * t.choice / total, 100.0 * t.paths / total, 100.0 * t.talk / total,
                100.0 * t.other / total);
    std::printf(
        "trips: %lld inside one part of a cluster, %lld by a cached path, %lld by a path found, %lld with "
        "no way; %d entrances, %zu paths cached\n",
        static_cast<long long>(trips.inside), static_cast<long long>(trips.cached), static_cast<long long>(trips.fresh),
        static_cast<long long>(trips.none), world.paths().entrances(), world.paths().cached());
    std::printf("someone's choice: %s\n", world.explain(0).c_str());
    std::printf("checksum %s\n", samebits::hex(world.checksum()).c_str());
    return 0;
}
