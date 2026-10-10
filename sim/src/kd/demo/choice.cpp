#include "kd/demo/choice.hpp"
namespace kd::demo {
namespace {
void parts(world::CraftReason& reason) {
    if (std::any_of(reason.parts.begin(), reason.parts.end(), [](auto x) { return x != 0; })) return;
    reason.parts[0] = std::max<std::int64_t>(0, 80 - reason.need_met) * reason.benefit * 10;
    reason.parts[2] = -reason.seconds / 60;
    reason.parts[1] = reason.score - reason.parts[0] - reason.parts[2];
}
}  // namespace
void ChoiceSet::add(world::CraftReason reason, Commit commit) {
    // A single beam for making and teaching: never retain more than eight known blueprints.
    if (reason.intended) {
        std::vector<std::pair<std::uint32_t, std::int64_t>> recipes;
        for (const auto& old : reasons_) {
            if (!old.intended) continue;
            auto found =
                std::find_if(recipes.begin(), recipes.end(), [&](const auto& r) { return r.first == old.recipe; });
            if (found == recipes.end())
                recipes.emplace_back(old.recipe, old.score);
            else
                found->second = std::max(found->second, old.score);
        }
        const bool present =
            std::any_of(recipes.begin(), recipes.end(), [&](const auto& r) { return r.first == reason.recipe; });
        if (!present && recipes.size() == 8) {
            const auto worst = std::min_element(recipes.rbegin(), recipes.rend(),
                                                [](const auto& a, const auto& b) { return a.second < b.second; });
            if (reason.score <= worst->second) return;
            const auto discarded = worst->first;
            for (std::size_t n = reasons_.size(); n-- > 0;)
                if (reasons_[n].intended && reasons_[n].recipe == discarded) {
                    reasons_.erase(reasons_.begin() + static_cast<std::ptrdiff_t>(n));
                    commits_.erase(commits_.begin() + static_cast<std::ptrdiff_t>(n));
                }
        }
    }
    KD_CHECK(reasons_.size() < kLimit, "The common chooser has at most thirty candidates");
    reasons_.push_back(std::move(reason));
    commits_.push_back(std::move(commit));
}
bool ChoiceSet::commit(world::Context& c, world::Beings::Handle h) {
    KD_CHECK(reasons_.size() >= 3, "A common choice keeps two real rejected candidates");
    std::vector<std::size_t> order;
    for (std::size_t n = 0; n < reasons_.size(); ++n) order.push_back(n);
    std::stable_sort(order.begin(), order.end(), [&](auto a, auto b) { return reasons_[a].score > reasons_[b].score; });
    const auto winner = order.front();
    std::vector<world::CraftReason> rejected;
    for (std::size_t n = 1; n < order.size(); ++n) rejected.push_back(reasons_[order[n]]);
    const auto id = Choices::keep(c, h, reasons_[winner], std::move(rejected), false);
    return commits_[winner](id);
}
world::CraftReason Choices::body(const world::Life& life, std::uint8_t goal) {
    world::CraftReason option;
    option.kind = 2;
    option.need = goal;
    option.action = goal;
    option.score = life.scores[goal];
    if (goal < 3) {
        option.benefit = life.benefit[goal];
        option.seconds = life.cost_seconds[goal];
        option.need_met = static_cast<std::uint8_t>(life.decision_needs[goal]);
        option.unavailable = life.unavailable[goal];
    }
    if (goal < 3 && !option.unavailable) {
        const auto urgency = goal == 2 && life.awake >= 129600 ? 200 : std::max<std::int64_t>(0, 80 - option.need_met);
        option.parts[0] = urgency * option.benefit * 10;
        option.parts[2] = -option.seconds / 60;
        option.parts[1] = option.score - option.parts[0] - option.parts[2];  // saved dream pull
    } else
        option.parts[1] = option.score;
    return option;
}
std::uint64_t Choices::keep(world::Context& c, world::Beings::Handle h, world::CraftReason winner,
                            std::vector<world::CraftReason> alternatives, bool add_body) {
    auto& raw = c.world().beings().raw();
    const auto& life = raw.get<world::Life>(h);
    auto& mind = raw.get<world::Knowledge>(h);
    for (auto& reason : alternatives) {
        if (reason.kind <= 1) {
            const auto* thermal = raw.try_get<world::Thermal>(h);
            reason.need_met = static_cast<std::uint8_t>(reason.need < 3    ? life.decision_needs[reason.need]
                                                        : reason.need == 4 ? (thermal ? thermal->warmth : 100)
                                                                           : mind.curiosity_need);
        }
        parts(reason);
    }
    if (add_body)
        for (std::uint8_t n = 0; n < 4; ++n) {
            auto option = body(life, n);
            parts(option);
            alternatives.push_back(option);
        }
    std::stable_sort(alternatives.begin(), alternatives.end(),
                     [](const auto& a, const auto& b) { return a.score > b.score; });
    parts(winner);
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
    const auto* kept = history.choices.find(choice);
    KD_CHECK(kept && kept->actor == w.beings().id_of(h), "choice does not identify this person's retained decision");
    mind.choice = choice;
    mind.reasons = kept->reasons;
}
}  // namespace kd::demo
