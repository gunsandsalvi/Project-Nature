// P5's run from the command line (RES-05): the toy world on one thread or several, printing each day's checksum
// and the run's digest, for the cloud's comparison of x86-64, arm64 under qemu and thread counts.
//   samebits [threads] [days]
// Pre-production code (research 00).
#include <cstdio>
#include <cstdlib>

#include "world.hpp"

int main(int argc, char** argv) {
    samebits::Settings settings;
    int days = 30;
    if (argc > 1) {
        settings.threads = std::atoi(argv[1]);
    }
    if (argc > 2) {
        days = std::atoi(argv[2]);
    }
    const auto sums = samebits::run(settings, days);
    for (std::size_t d = 0; d < sums.size(); ++d) {
        std::printf("day %zu %s\n", d + 1, samebits::hex(sums[d]).c_str());
    }
    std::printf("digest %s\n", samebits::digest(sums).c_str());
    return 0;
}
