// P6's actions (MND-09): the 50 things people do, as data: what each is done to, when it can be done, how long it
// takes, what it does for each need when it ends, its effort and risk, and the trait that draws a person to it.
// Choosing reads only this table, so a new action needs no code. Pre-production code (research 00).
#pragma once

#include <array>
#include <cstdint>

#include "land.hpp"

namespace minds {

// The needs (BIO-09, MND-07), each from 0, unmet, to 100, met.
enum Need : std::uint8_t { kHunger, kThirst, kRest, kWarmth, kSafety, kBelonging, kStatus, kCuriosity, kLove };
constexpr int kNeeds = 9;
// The traits, each from 0 to 1 (MND-20).
enum Trait : std::uint8_t { kOpenness, kConscientious, kOutgoing, kAgreeable, kNervous };
constexpr int kTraits = 5;

// What an action is done to.
enum class Target : std::uint8_t {
    kSelf,       // where they are
    kCamp,       // their band's camp
    kSpot,       // a known spot of the action's kind
    kPerson,     // someone within 20 m
    kLeader,     // their band's leader, wherever they are
    kFar,        // a place they don't know, a few hundred metres off
    kNear,       // somewhere close by
    kOtherCamp,  // another band's camp
};

// When an action can be done.
enum Gate : std::uint16_t {
    kByDay = 1U << 0U,
    kAtNight = 1U << 1U,
    kAdults = 1U << 2U,
    kChildren = 1U << 3U,
    kFireLit = 1U << 4U,
    kCompany = 1U << 5U,
    kHasFood = 1U << 6U,
    kHasWater = 1U << 7U,
    kHasWood = 1U << 8U,
    kHasFlint = 1U << 9U,
    kHasGame = 1U << 10U,
    kStoreFood = 1U << 11U,
    kStoreWood = 1U << 12U,
    kEvening = 1U << 13U,
    kWithChild = 1U << 14U,  // with a child within 20 m, rather than whoever is best known
};

// What lands when an action ends (TIM-17).
enum class Result : std::uint8_t {
    kNone,
    kEatCarried,
    kEatStore,
    kEatSpot,
    kDrinkCarried,
    kDrinkSpot,
    kGather,
    kHunt,
    kFetchWater,
    kFetchWood,
    kFetchFlint,
    kStore,
    kTendFire,
    kButcher,
    kKnap,
    kShareFood,
    kTalk,
    kExplore,
    kSleep,
};

// The part of the day an action suits, for habits.
enum class Habit : std::uint8_t { kAny, kMorning, kMidday, kEvening, kNight };

struct Action {
    const char* name;
    Target target;
    Kind spot;
    std::uint16_t gates;
    std::int16_t minutes;  // the work's length, the walk there aside
    std::int16_t spread;   // up to this many minutes more or less, by chance
    std::array<std::int8_t, kNeeds> gain;
    float effort;  // 0 resting to 1 hard work, which tires and hungers
    float risk;    // the chance of harm that a nervous person weighs
    Result result;
    Trait trait;  // the trait that draws a person to it
    float pull;   // how much, -1 to 1
    Habit habit;
};

constexpr int kActions = 50;
//                              H   T   R   W   S   B  St   C   L
using G = std::array<std::int8_t, kNeeds>;

constexpr std::array<Action, kActions> kTable = {{
    {"eat what they carry", Target::kSelf, Kind::kWater, kHasFood, 10, 5, G{35, 0, 0, 0, 0, 0, 0, 0, 0}, 0.05F, 0.0F,
     Result::kEatCarried, kConscientious, 0.0F, Habit::kAny},
    {"eat at camp", Target::kCamp, Kind::kWater, kStoreFood, 20, 10, G{45, 0, 0, 0, 0, 5, 0, 0, 0}, 0.05F, 0.0F,
     Result::kEatStore, kOutgoing, 0.2F, Habit::kEvening},
    {"eat berries", Target::kSpot, Kind::kBerries, kByDay, 30, 15, G{30, 0, 0, 0, 0, 0, 0, 0, 0}, 0.2F, 0.0F,
     Result::kEatSpot, kOpenness, 0.0F, Habit::kMidday},
    {"eat nuts", Target::kSpot, Kind::kNuts, kByDay, 40, 15, G{35, 0, 0, 0, 0, 0, 0, 0, 0}, 0.25F, 0.0F,
     Result::kEatSpot, kOpenness, 0.0F, Habit::kMidday},
    {"dig and eat roots", Target::kSpot, Kind::kRoots, kByDay, 50, 20, G{30, 0, 0, 0, 0, 0, 0, 0, 0}, 0.4F, 0.0F,
     Result::kEatSpot, kConscientious, 0.1F, Habit::kAny},
    {"drink what they carry", Target::kSelf, Kind::kWater, kHasWater, 5, 2, G{0, 40, 0, 0, 0, 0, 0, 0, 0}, 0.0F, 0.0F,
     Result::kDrinkCarried, kConscientious, 0.0F, Habit::kAny},
    {"drink at water", Target::kSpot, Kind::kWater, 0, 5, 3, G{0, 60, 0, 0, 0, 0, 0, 0, 0}, 0.05F, 0.0F,
     Result::kDrinkSpot, kConscientious, 0.0F, Habit::kAny},
    {"fetch water", Target::kSpot, Kind::kWater, kByDay, 10, 5, G{0, 20, 0, 0, 5, 0, 3, 0, 0}, 0.25F, 0.0F,
     Result::kFetchWater, kConscientious, 0.3F, Habit::kMorning},
    {"sleep at camp", Target::kCamp, Kind::kWater, kAtNight, 420, 60, G{0, 0, 90, 10, 10, 0, 0, 0, 0}, 0.0F, 0.0F,
     Result::kSleep, kNervous, 0.0F, Habit::kNight},
    {"nap at camp", Target::kCamp, Kind::kWater, kByDay, 60, 30, G{0, 0, 30, 0, 0, 0, 0, 0, 0}, 0.0F, 0.0F,
     Result::kNone, kConscientious, -0.3F, Habit::kMidday},
    {"rest where they are", Target::kSelf, Kind::kWater, 0, 30, 15, G{0, 0, 15, 0, 0, 0, 0, 0, 0}, 0.0F, 0.0F,
     Result::kNone, kConscientious, -0.2F, Habit::kAny},
    {"warm at the fire", Target::kCamp, Kind::kWater, kFireLit, 40, 20, G{0, 0, 5, 50, 5, 5, 0, 0, 0}, 0.0F, 0.0F,
     Result::kNone, kOutgoing, 0.1F, Habit::kEvening},
    {"tend the fire", Target::kCamp, Kind::kWater, kStoreWood, 10, 5, G{0, 0, 0, 5, 5, 0, 3, 0, 0}, 0.1F, 0.0F,
     Result::kTendFire, kConscientious, 0.4F, Habit::kEvening},
    {"gather berries to bring back", Target::kSpot, Kind::kBerries, kByDay | kAdults, 90, 30,
     G{5, 0, 0, 0, 3, 3, 5, 0, 0}, 0.4F, 0.0F, Result::kGather, kConscientious, 0.4F, Habit::kMorning},
    {"gather nuts", Target::kSpot, Kind::kNuts, kByDay | kAdults, 120, 30, G{5, 0, 0, 0, 3, 3, 5, 0, 0}, 0.4F, 0.0F,
     Result::kGather, kConscientious, 0.4F, Habit::kMorning},
    {"dig roots to bring back", Target::kSpot, Kind::kRoots, kByDay | kAdults, 120, 30, G{0, 0, 0, 0, 3, 3, 5, 0, 0},
     0.6F, 0.0F, Result::kGather, kConscientious, 0.4F, Habit::kMorning},
    {"hunt", Target::kSpot, Kind::kGame, kByDay | kAdults, 240, 60, G{0, 0, 0, 0, 0, 5, 15, 5, 0}, 0.9F, 0.2F,
     Result::kHunt, kNervous, -0.4F, Habit::kMorning},
    {"butcher the kill", Target::kCamp, Kind::kWater, kHasGame, 60, 20, G{10, 0, 0, 0, 5, 5, 10, 0, 0}, 0.5F, 0.0F,
     Result::kButcher, kConscientious, 0.3F, Habit::kAny},
    {"fetch firewood", Target::kSpot, Kind::kWood, kByDay, 60, 20, G{0, 0, 0, 3, 5, 0, 3, 0, 0}, 0.6F, 0.0F,
     Result::kFetchWood, kConscientious, 0.3F, Habit::kAny},
    {"fetch flint", Target::kSpot, Kind::kFlint, kByDay | kAdults, 60, 20, G{0, 0, 0, 0, 0, 0, 3, 5, 0}, 0.4F, 0.0F,
     Result::kFetchFlint, kOpenness, 0.2F, Habit::kAny},
    {"knap flint", Target::kCamp, Kind::kWater, kHasFlint | kAdults, 45, 15, G{0, 0, 0, 0, 3, 0, 8, 5, 0}, 0.3F, 0.05F,
     Result::kKnap, kConscientious, 0.3F, Habit::kMidday},
    {"scrape a hide", Target::kCamp, Kind::kWater, kByDay | kAdults, 90, 30, G{0, 0, 0, 3, 0, 0, 6, 0, 0}, 0.5F, 0.0F,
     Result::kNone, kConscientious, 0.3F, Habit::kMidday},
    {"twist cord", Target::kCamp, Kind::kWater, 0, 60, 20, G{0, 0, 0, 0, 0, 3, 4, 2, 0}, 0.2F, 0.0F, Result::kNone,
     kConscientious, 0.2F, Habit::kEvening},
    {"cook at the fire", Target::kCamp, Kind::kWater, kFireLit | kStoreFood, 45, 15, G{25, 0, 0, 5, 0, 8, 3, 0, 0},
     0.2F, 0.0F, Result::kEatStore, kAgreeable, 0.2F, Habit::kEvening},
    {"mend the shelter", Target::kCamp, Kind::kWater, kByDay | kAdults, 90, 30, G{0, 0, 0, 5, 15, 0, 5, 0, 0}, 0.6F,
     0.0F, Result::kNone, kConscientious, 0.4F, Habit::kAny},
    {"store what they carry", Target::kCamp, Kind::kWater, kHasFood, 10, 5, G{0, 0, 0, 0, 8, 3, 4, 0, 0}, 0.05F, 0.0F,
     Result::kStore, kConscientious, 0.4F, Habit::kAny},
    {"stack the firewood", Target::kCamp, Kind::kWater, kHasWood, 10, 5, G{0, 0, 0, 3, 5, 0, 3, 0, 0}, 0.1F, 0.0F,
     Result::kStore, kConscientious, 0.3F, Habit::kAny},
    {"chat", Target::kPerson, Kind::kWater, kCompany, 20, 10, G{0, 0, 0, 0, 0, 20, 0, 3, 3}, 0.0F, 0.0F, Result::kTalk,
     kOutgoing, 0.5F, Habit::kAny},
    {"tell news", Target::kPerson, Kind::kWater, kCompany, 15, 5, G{0, 0, 0, 0, 0, 10, 5, 3, 0}, 0.0F, 0.0F,
     Result::kTalk, kOutgoing, 0.4F, Habit::kAny},
    {"groom kin", Target::kPerson, Kind::kWater, kCompany, 30, 10, G{0, 0, 3, 0, 3, 15, 0, 0, 10}, 0.0F, 0.0F,
     Result::kTalk, kAgreeable, 0.4F, Habit::kAny},
    {"play with a child", Target::kPerson, Kind::kWater, kCompany | kAdults | kWithChild, 30, 10,
     G{0, 0, 0, 0, 0, 5, 0, 3, 15}, 0.1F, 0.0F, Result::kTalk, kAgreeable, 0.4F, Habit::kEvening},
    {"care for a child", Target::kPerson, Kind::kWater, kCompany | kAdults | kWithChild, 30, 10,
     G{0, 0, 0, 0, 5, 3, 3, 0, 10}, 0.1F, 0.0F, Result::kTalk, kAgreeable, 0.5F, Habit::kAny},
    {"court", Target::kPerson, Kind::kWater, kCompany | kAdults, 30, 15, G{0, 0, 0, 0, 0, 5, 3, 0, 25}, 0.0F, 0.05F,
     Result::kTalk, kOutgoing, 0.4F, Habit::kEvening},
    {"share food", Target::kPerson, Kind::kWater, kCompany | kHasFood, 10, 5, G{0, 0, 0, 0, 0, 10, 8, 0, 5}, 0.0F, 0.0F,
     Result::kShareFood, kAgreeable, 0.6F, Habit::kAny},
    {"teach a craft", Target::kPerson, Kind::kWater, kCompany | kAdults, 60, 20, G{0, 0, 0, 0, 0, 5, 10, 3, 3}, 0.2F,
     0.0F, Result::kTalk, kConscientious, 0.3F, Habit::kMidday},
    {"watch someone work", Target::kPerson, Kind::kWater, kCompany, 30, 10, G{0, 0, 3, 0, 0, 3, 0, 15, 0}, 0.0F, 0.0F,
     Result::kTalk, kOpenness, 0.4F, Habit::kAny},
    {"visit another band", Target::kOtherCamp, Kind::kWater, kByDay | kAdults, 120, 30, G{0, 0, 0, 0, -5, 20, 5, 15, 5},
     0.4F, 0.05F, Result::kTalk, kOutgoing, 0.5F, Habit::kMorning},
    {"go with the leader", Target::kLeader, Kind::kWater, kByDay, 60, 30, G{0, 0, 0, 0, 10, 10, 3, 0, 0}, 0.2F, 0.0F,
     Result::kNone, kAgreeable, 0.3F, Habit::kAny},
    {"sing and dance at the fire", Target::kCamp, Kind::kWater, kFireLit | kCompany | kEvening, 60, 30,
     G{0, 0, -5, 5, 0, 20, 3, 0, 5}, 0.3F, 0.0F, Result::kNone, kOutgoing, 0.5F, Habit::kEvening},
    {"explore", Target::kFar, Kind::kWater, kByDay, 60, 30, G{0, 0, 0, 0, -3, 0, 3, 30, 0}, 0.5F, 0.1F,
     Result::kExplore, kOpenness, 0.6F, Habit::kMorning},
    {"try something new", Target::kSelf, Kind::kWater, kByDay, 40, 20, G{0, 0, 0, 0, 0, 0, 3, 20, 0}, 0.2F, 0.05F,
     Result::kNone, kOpenness, 0.6F, Habit::kAny},
    {"wander nearby", Target::kNear, Kind::kWater, 0, 30, 15, G{0, 0, 5, 0, 0, 0, 0, 10, 0}, 0.1F, 0.0F,
     Result::kExplore, kOpenness, 0.2F, Habit::kAny},
    {"keep watch", Target::kSelf, Kind::kWater, 0, 60, 20, G{0, 0, 0, 0, 25, 0, 5, 0, 0}, 0.1F, 0.0F, Result::kNone,
     kNervous, 0.5F, Habit::kNight},
    {"play", Target::kSelf, Kind::kWater, kChildren, 45, 15, G{0, 0, 0, 0, 0, 10, 0, 15, 0}, 0.2F, 0.0F, Result::kNone,
     kOutgoing, 0.3F, Habit::kAny},
    {"sit and think", Target::kSelf, Kind::kWater, 0, 30, 15, G{0, 0, 5, 0, 0, 0, 0, 5, 0}, 0.0F, 0.0F, Result::kNone,
     kOpenness, 0.3F, Habit::kAny},
    {"tell stories at the fire", Target::kCamp, Kind::kWater, kFireLit | kCompany | kEvening, 45, 15,
     G{0, 0, 0, 3, 0, 15, 8, 3, 0}, 0.0F, 0.0F, Result::kNone, kOutgoing, 0.4F, Habit::kEvening},
    {"gather herbs", Target::kSpot, Kind::kRoots, kByDay, 60, 20, G{0, 0, 0, 0, 5, 0, 3, 5, 0}, 0.3F, 0.0F,
     Result::kNone, kOpenness, 0.2F, Habit::kAny},
    {"fish at the river", Target::kSpot, Kind::kWater, kByDay | kAdults, 120, 40, G{5, 0, 0, 0, 0, 0, 5, 3, 0}, 0.4F,
     0.0F, Result::kGather, kConscientious, 0.2F, Habit::kMorning},
    {"set snares", Target::kSpot, Kind::kGame, kByDay | kAdults, 60, 20, G{0, 0, 0, 0, 0, 0, 3, 3, 0}, 0.4F, 0.0F,
     Result::kNone, kConscientious, 0.2F, Habit::kAny},
    {"go back to camp", Target::kCamp, Kind::kWater, 0, 5, 0, G{0, 0, 0, 0, 10, 5, 0, 0, 0}, 0.0F, 0.0F, Result::kNone,
     kNervous, 0.2F, Habit::kEvening},
}};

}  // namespace minds
