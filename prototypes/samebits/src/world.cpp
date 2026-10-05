// P5's toy world (A3.4, A3.5): see world.hpp. Pre-production code (research 00).
#include "world.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <thread>

#include "chance.hpp"
#include "maths.hpp"

namespace samebits {

namespace {

constexpr double kPi = 3.14159265358979311600e+00;
constexpr double kTwoPi = 6.28318530717958623200e+00;
constexpr double kCellSide = World::kSide / World::kCells;

// A place wrapped onto the torus.
double wrap(double v) {
    return v - World::kSide * std::floor(v / World::kSide);
}

int cell(double v) {
    return std::clamp(static_cast<int>(v / kCellSide), 0, World::kCells - 1);
}

// Runs work on every chunk of count things, on one thread or several, each chunk wholly on one thread: chunk c goes
// to thread c mod threads. The work writes only its own chunk's things, so the result never depends on the threads.
void chunks(int count, int threads, const std::function<void(int)>& work) {
    const int n = (count + World::kChunk - 1) / World::kChunk;
    if (threads <= 1) {
        for (int c = 0; c < n; ++c) {
            work(c);
        }
        return;
    }
    std::vector<std::thread> pool;
    pool.reserve(static_cast<std::size_t>(threads));
    for (int t = 0; t < threads; ++t) {
        pool.emplace_back([&work, t, n, threads] {
            for (int c = t; c < n; c += threads) {
                work(c);
            }
        });
    }
    for (auto& thread : pool) {
        thread.join();
    }
}

}  // namespace

World::World(const Settings& settings) : settings_(settings) {
    const auto count = static_cast<std::size_t>(settings_.walkers);
    walkers_.resize(count);
    food_.assign(static_cast<std::size_t>(kCells) * static_cast<std::size_t>(kCells), 4.0);
    for (std::size_t i = 0; i < count; ++i) {
        Walker& w = walkers_[i];
        const auto id = static_cast<std::uint64_t>(i);
        w.x = chance(settings_.seed, id, 0, Purpose::kStart, 0) * kSide;
        w.y = chance(settings_.seed, id, 0, Purpose::kStart, 1) * kSide;
        w.heading = (chance(settings_.seed, id, 0, Purpose::kStart, 2) - 0.5) * kTwoPi;
        w.speed = 0.5 + chance(settings_.seed, id, 0, Purpose::kStart, 3);
        w.energy = 20.0;
        push(static_cast<std::int64_t>(std::floor(chance(settings_.seed, id, 0, Purpose::kWait) * 30.0)),
             static_cast<std::uint32_t>(i));
    }
}

void World::push(std::int64_t tick, std::uint32_t id) {
    walkers_[id].next = tick;
    queue_.emplace_back(tick, id);
    std::push_heap(queue_.begin(), queue_.end(), std::greater<>());
}

void World::run_day() {
    const std::int64_t end = (static_cast<std::int64_t>(day_) + 1) * kTicksPerDay;
    while (!queue_.empty() && queue_.front().first < end) {
        std::pop_heap(queue_.begin(), queue_.end(), std::greater<>());
        const auto [tick, id] = queue_.back();
        queue_.pop_back();
        tick_ = tick;
        event(id);
    }
    tick_ = end;
    evening();
    ++day_;
}

// A walker's event: it turns a little, walks until its next event, tires, eats if there is food where it stops.
void World::event(std::uint32_t id) {
    Walker& w = walkers_[id];
    const auto t = static_cast<std::uint64_t>(tick_);
    w.heading += (chance(settings_.seed, id, t, Purpose::kTurn) - 0.5) * 1.2;
    if (w.heading > kPi) {
        w.heading -= kTwoPi;
    } else if (w.heading < -kPi) {
        w.heading += kTwoPi;
    }
    const double wait = 1.0 + std::floor(chance(settings_.seed, id, t, Purpose::kWait) * 30.0);
    const double step = w.speed * (1.0 + 0.25 * mood_) * wait;
    w.x = wrap(w.x + cosine(w.heading) * step);
    w.y = wrap(w.y + sine(w.heading) * step);
    w.energy *= exponent(-0.0004 * wait);
    const std::size_t at =
        static_cast<std::size_t>(cell(w.y)) * static_cast<std::size_t>(kCells) + static_cast<std::size_t>(cell(w.x));
    double& food = food_[at];
    if (food > 0.5 && chance(settings_.seed, id, t, Purpose::kEat) < 0.3) {
        const double bite = std::min(food, 2.0);
        food -= bite;
        w.energy += bite;
    }
    push(tick_ + static_cast<std::int64_t>(wait), id);
}

// The evening's work, in chunks: each walker's pace from its energy, the food regrowing cell by cell, and the
// day's sums, gathered chunk by chunk in order into the next day's mood.
void World::evening() {
    const int count = settings_.walkers;
    const std::size_t pieces = static_cast<std::size_t>((count + kChunk - 1) / kChunk);
    std::vector<double> energy(pieces, 0.0);
    std::vector<double> facing(pieces, 0.0);
    chunks(count, settings_.threads, [this, count, &energy, &facing](int c) {
        double sum = 0.0;
        double turn = 0.0;
        for (int i = c * kChunk; i < std::min(count, (c + 1) * kChunk); ++i) {
            Walker& w = walkers_[static_cast<std::size_t>(i)];
            w.energy = power(w.energy + 1.0, 0.999) - 1.0;
            w.speed = 0.4 + 0.6 * (1.0 - exponent(-0.05 * w.energy));
            sum += logarithm(1.0 + w.energy);
            turn += sine(w.heading);
        }
        energy[static_cast<std::size_t>(c)] = sum;
        facing[static_cast<std::size_t>(c)] = turn;
    });
    const int cells = kCells * kCells;
    const double season = sine(0.2 * static_cast<double>(day_));
    chunks(cells, settings_.threads, [this, cells, season](int c) {
        for (int i = c * kChunk; i < std::min(cells, (c + 1) * kChunk); ++i) {
            double& food = food_[static_cast<std::size_t>(i)];
            food += 0.3 * (1.0 - food / 8.0) * (0.75 + 0.25 * season) + 0.01 * cosine(static_cast<double>(i));
        }
    });
    double sum = 0.0;
    double turn = 0.0;
    for (std::size_t c = 0; c < pieces; ++c) {
        sum += energy[c];
        turn += facing[c];
    }
    const auto n = static_cast<double>(count);
    mood_ = (turn / n) * (sum / n);
}

std::uint64_t World::checksum() const {
    std::uint64_t h = kHashStart;
    for (const Walker& w : walkers_) {
        hash_into(&h, w.x);
        hash_into(&h, w.y);
        hash_into(&h, w.heading);
        hash_into(&h, w.speed);
        hash_into(&h, w.energy);
        hash_into(&h, static_cast<std::uint64_t>(w.next));
    }
    for (const double food : food_) {
        hash_into(&h, food);
    }
    hash_into(&h, mood_);
    hash_into(&h, static_cast<std::uint64_t>(tick_));
    return h;
}

std::vector<std::uint64_t> run(const Settings& settings, int days) {
    World world(settings);
    std::vector<std::uint64_t> sums;
    sums.reserve(static_cast<std::size_t>(days));
    for (int d = 0; d < days; ++d) {
        world.run_day();
        sums.push_back(world.checksum());
    }
    return sums;
}

}  // namespace samebits
