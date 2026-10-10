#include "kd/demo/choice.hpp"
namespace kd::demo {
std::uint64_t Choices::keep(world::Context& c, world::Beings::Handle h, world::CraftReason winner,
                            std::vector<world::CraftReason> alternatives) {
    auto& raw = c.world().beings().raw();
    const auto& life = raw.get<world::Life>(h);
    auto& mind = raw.get<world::Knowledge>(h);
    for (auto& reason : alternatives)
        reason.need_met =
            static_cast<std::uint8_t>(reason.need < 3 ? life.decision_needs[reason.need] : mind.curiosity_need);
    for (std::uint8_t n = 0; n < 4; ++n) {
        world::CraftReason option;
        option.kind = 2;
        option.need = n;
        option.action = n;
        option.score = life.scores[n];
        if (n < 3) {
            option.benefit = life.benefit[n];
            option.seconds = life.cost_seconds[n];
            option.need_met = static_cast<std::uint8_t>(life.decision_needs[n]);
            option.unavailable = life.unavailable[n];
        }
        alternatives.push_back(option);
    }
    std::stable_sort(alternatives.begin(), alternatives.end(),
                     [](const auto& a, const auto& b) { return a.score > b.score; });
    mind.reasons = {std::move(winner), alternatives[0], alternatives[1]};
    c.touch(raw.get<Home>(h).camp);
    auto& history = raw.get<world::CraftHistory>(c.world().beings().handle(raw.get<Home>(h).camp));
    mind.choice = history.next_choice++;
    history.choices.push_back({mind.choice, c.now(), c.world().beings().id_of(h), mind.reasons});
    c.record(217, c.world().beings().id_of(h).value, mind.choice);
    return mind.choice;
}
void Choices::restore(world::World& w, world::Beings::Handle h, std::uint64_t choice) {
    auto& raw = w.beings().raw();
    auto& mind = raw.get<world::Knowledge>(h);
    mind.reasons.clear();
    mind.choice = 0;
    if (!choice) return;
    const auto& history = raw.get<world::CraftHistory>(w.beings().handle(raw.get<Home>(h).camp));
    KD_CHECK(choice <= history.choices.size() && history.choices[choice - 1].id == choice &&
                 history.choices[choice - 1].actor == w.beings().id_of(h),
             "A resumed plan retains its own choice");
    mind.choice = choice;
    mind.reasons = history.choices[choice - 1].reasons;
}
}  // namespace kd::demo
