// P6 A thousand minds (IMPLEMENTATION α0.4b, A3.3, A3.9, A11): a thousand people in 40 bands live on the land,
// each a full simple mind: needs running down at their own rates; at the end of each activity, a choice among about
// 50 actions scored by response curves, the top reasons kept; talk that passes places, opinions and news; and trips
// by the paths in levels. Work happens only at events (TIM-17): the activities ending in each 5 minutes of game time
// land in order, then their people choose again in parallel, in fixed pieces, reading the world as it stood; what
// they do to each other lands, in order, when their activities end. So one thread and four end every day the same.
// Pre-production code (research 00).
#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "actions.hpp"
#include "land.hpp"
#include "paths.hpp"
#include "pool.hpp"

namespace minds {

constexpr std::int64_t kMinute = 60;  // the clock counts game seconds (A3.3)
constexpr std::int64_t kHour = 60 * kMinute;
constexpr std::int64_t kDay = 24 * kHour;
constexpr int kYear = 60;  // days in a game year (TIM-18)
// the span of game time whose ending activities are taken together; no activity is shorter
constexpr std::int64_t kWindow = 5 * kMinute;
// the most a mind holds (MND-14): a few hundred places, 150 people, about 200 memories
constexpr int kPlacesHeld = 256;
constexpr int kTiesHeld = 150;
constexpr int kMemoriesHeld = 200;
constexpr int kTopics = 3;  // the most one talk passes on

struct Settings {
    std::uint64_t seed = 1;
    int bands = 40;
    int band_size = 25;
    int threads = 1;
};

// A place someone knows (MND-28): what is there, how much they believe it holds, when they learned it, and from
// whom (MND-23).
struct Place {
    std::int32_t cell = 0;
    std::int32_t spot = -1;
    std::int32_t told_by = -1;  // -1 when they saw it themselves
    std::int32_t day = 0;
    float amount = 0.0F;
    Kind kind = Kind::kWater;
};

// Someone they know (MND-24): how much they like them, -1 to 1, and how well they know them, 0 to 1.
struct Tie {
    std::int32_t other = 0;
    float opinion = 0.0F;
    float familiarity = 0.0F;
};

// Something that happened to them (MND-18).
struct Memory {
    std::int64_t tick = 0;
    std::int32_t who = -1;
    std::int16_t what = 0;  // the action
    float strength = 0.0F;  // how it felt, -1 to 1
};

// What they pass on in a talk (MND-33): a place, an opinion of someone, or their latest news.
struct Topic {
    std::uint8_t kind = 0;  // 1 a place, 2 an opinion, 3 news; 0 nothing
    std::int32_t index = 0;
};

// What someone is doing (TIM-17), and why (MND-09, PRN-13).
struct Activity {
    std::int64_t start = 0;
    std::int64_t end = 0;       // when it ends, cut short or not
    std::int64_t full_end = 0;  // when it would end, uncut
    std::int32_t from = 0;      // cells
    std::int32_t to = 0;
    std::int32_t target = -1;  // a spot, a person or a camp, by the action's target
    float walk = 0.0F;         // seconds of walking before the work
    std::int16_t action = 0;
    std::array<std::uint8_t, 3> reasons{255, 255, 255};  // the score's parts that most put it ahead; 255 none
    std::array<std::int16_t, 2> beaten{-1, -1};          // the two best options it beat; -1 none
    std::array<Topic, kTopics> topics{};
};

struct Person {
    std::int32_t band = 0;
    float age = 0.0F;
    std::array<float, kNeeds> need{};
    std::int64_t needs_at = 0;  // the tick the needs were last brought up to
    std::array<float, kTraits> trait{};
    float mood = 0.0F;
    // carried
    float food = 0.0F;
    float water = 0.0F;
    float wood = 0.0F;
    float flint = 0.0F;
    float game = 0.0F;
    std::int32_t cell = 0;  // where they stood when the activity began
    Activity now;
    int places = 0;
    int ties = 0;
    int memories = 0;
    int memory_next = 0;
    std::array<Place, kPlacesHeld> place{};
    std::array<Tie, kTiesHeld> tie{};
    std::array<Memory, kMemoriesHeld> memory{};
};

// The time each part of the minds took, in seconds of each thread's time, summed.
struct Times {
    double land = 0.0;    // activities ending: needs brought up to date, results landing
    double choice = 0.0;  // the options found and scored, one picked
    double paths = 0.0;   // trips
    double talk = 0.0;    // someone to talk to and what to pass on, and the talk heard
    double other = 0.0;   // the snapshot of where people stand, the queue, the hourly and daily passes
};

class World {
public:
    explicit World(const Settings& settings);
    ~World();
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World&&) = delete;
    World& operator=(World&&) = delete;

    // One game day: every activity ending in it, five minutes at a time, and the day's passes.
    void run_day();
    // The whole state's hash: every person, every spot and every camp.
    [[nodiscard]] std::uint64_t checksum() const;
    [[nodiscard]] int day() const { return day_; }
    [[nodiscard]] const Times& times() const { return times_; }
    [[nodiscard]] std::int64_t decisions() const { return decisions_; }
    // Activities set to end early, when a need they don't meet would fall below 20 first (TIM-17).
    [[nodiscard]] std::int64_t cut_short() const;
    [[nodiscard]] TripCounts trips() const;
    [[nodiscard]] const std::vector<Person>& people() const { return people_; }
    [[nodiscard]] const Land& land() const { return land_; }
    [[nodiscard]] const Paths& paths() const { return paths_; }
    // What a person is doing and why, in words (PRN-13).
    [[nodiscard]] std::string explain(int person) const;

private:
    struct Scratch;
    struct Option;

    void window();
    void land_activity(std::int32_t id);
    void decide(std::int32_t id, Scratch* s);
    void snapshot();
    void hourly();
    void daily();
    void choose_leaders();
    void see(Person* p, std::int32_t spot) const;
    void bring_needs(Person* p, std::int64_t to, float effort, bool asleep) const;
    void learn_place(Person* p, const Place& place) const;
    void remember(Person* p, std::int64_t tick, std::int32_t who, int what, float strength) const;
    void hear(std::int32_t speaker, std::int32_t listener);
    [[nodiscard]] std::int32_t cell_now(const Person& p, std::int64_t t) const;
    void near(int cell, double within, std::int32_t self, std::vector<std::int32_t>* out) const;

    Settings settings_;
    Land land_;
    Paths paths_;
    Pool pool_;
    std::vector<Person> people_;
    std::vector<std::int32_t> leader_;                     // each band's leader
    std::vector<std::array<std::int32_t, 3>> neighbours_;  // each band's three nearest other bands
    std::vector<std::vector<std::int32_t>> queue_;         // people by the window their activity ends in
    std::vector<std::int32_t> where_;                      // where each stood at the window's start
    std::vector<std::int32_t> grid_start_;                 // people by grid square of 32 m, from where_
    std::vector<std::int32_t> grid_;
    std::vector<std::int32_t> grid_fill_;
    std::vector<std::int32_t> found_;  // spots an explorer comes upon
    std::vector<std::unique_ptr<Scratch>> scratch_;
    double hear_time_ = 0.0;
    std::int64_t now_ = 0;
    int day_ = 0;
    std::int64_t decisions_ = 0;
    Times times_;
};

// A run's checksums at the end of each day.
std::vector<std::uint64_t> run(const Settings& settings, int days);

}  // namespace minds
