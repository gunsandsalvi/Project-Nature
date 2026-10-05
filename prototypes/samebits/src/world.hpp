// P5 The same bits (IMPLEMENTATION α0.4a, A3.4, A3.5): a toy world that does what the real simulation will, in
// small: walkers on a torus act at events, in the order of their ticks; every draw is keyed chance; their turns,
// steps and tiredness use our own sine, cosine, exponent, logarithm and power; and each evening's work is done in
// fixed chunks on one thread or several, its sums gathered in chunk order. The whole state is hashed at the end of
// every game day: on every chip and at any number of threads, the hashes must be the same (RES-05, TIM-16).
// Pre-production code (research 00).
#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "hash.hpp"

namespace samebits {

struct Settings {
    std::uint64_t seed = 1;
    int walkers = 4096;
    int threads = 1;
};

struct Walker {
    double x = 0.0;        // metres east on the torus
    double y = 0.0;        // metres north
    double heading = 0.0;  // radians from east, within half a turn either way
    double speed = 0.0;    // metres a minute
    double energy = 0.0;
    std::int64_t next = 0;  // the tick of its next event
};

class World {
public:
    static constexpr std::int64_t kTicksPerDay = 1440;  // a game day, one tick a minute
    static constexpr double kSide = 1024.0;             // the torus, metres a side
    static constexpr int kCells = 64;                   // the food's grid, cells a side
    static constexpr int kChunk = 256;                  // walkers or cells in one piece of the evening's work

    explicit World(const Settings& settings);

    // One game day: every event before its end, in tick order (a tie by walker), then the evening's work.
    void run_day();
    // The whole state's hash: every walker in order, every cell of food, the day's mood and the tick.
    [[nodiscard]] std::uint64_t checksum() const;
    [[nodiscard]] int day() const { return day_; }
    [[nodiscard]] const std::vector<Walker>& walkers() const { return walkers_; }

private:
    void event(std::uint32_t id);
    void evening();
    void push(std::int64_t tick, std::uint32_t id);

    Settings settings_;
    std::vector<Walker> walkers_;
    std::vector<double> food_;
    std::vector<std::pair<std::int64_t, std::uint32_t>> queue_;  // a heap, earliest first
    std::int64_t tick_ = 0;
    int day_ = 0;
    // from the evening's sums, it sets how fast walkers go the next day, so sums gathered in another order would
    // change the world
    double mood_ = 0.0;
};

// The checksums at the end of each of a run's days.
std::vector<std::uint64_t> run(const Settings& settings, int days);

}  // namespace samebits
