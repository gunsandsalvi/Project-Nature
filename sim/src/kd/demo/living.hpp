#pragma once
#include "kd/demo/living_rules.hpp"
#include "kd/world/world.hpp"
namespace kd::demo {
class Living final : public world::System {
public:
    explicit Living(world::World& w);
    [[nodiscard]] std::string_view name() const override { return "living"; }
    void handle(world::Context& c, const event::Event& e) override;
    void opened(world::World& w) override;
    void item_changed(world::Context& c, ecs::Id id) override;
    [[nodiscard]] bool serial_windows(const world::World& w) const override {
        return !w.beings().raw().view<world::Work>().empty() || !w.things().raw().view<world::Fire>().empty() ||
               !w.things().raw().view<world::HeatTimer>().empty();
    }
    void start(world::World& w);
    static constexpr std::uint32_t kPlaceDream = 2;
    static constexpr std::uint32_t kIdeaDream = 3;
    [[nodiscard]] static std::string dream_limit(const world::World& w, ecs::Id person, time::Seconds at = -1);
    static constexpr std::int64_t kDreamPull = 60;
    static constexpr time::Seconds kDreamLife = 3 * time::kDay;
    [[nodiscard]] static std::int64_t night(time::Seconds t) {
        return t / time::kDay + (t % time::kDay + 18 * time::kHour) / time::kDay - 1;
    }
    [[nodiscard]] static std::string dream_problem(const world::World& w, ecs::Id person, std::int64_t subject,
                                                   time::Seconds at = -1);
    void command(world::Context& c, const world::Command& cmd) override;
    [[nodiscard]] std::int64_t reach() const override { return 10000; }
    void bounds(const world::World& w, time::Seconds a, time::Seconds b, std::span<const ecs::Id> owners,
                std::vector<world::Bound>& out) const override;
    void near(const world::World& w, time::Seconds a, time::Seconds b, const world::Bound& bound,
              std::vector<ecs::Id>& out) const override;
    [[nodiscard]] static std::array<std::int64_t, 3> needs(const world::Life& life);
    [[nodiscard]] world::Life sample(world::Life life, const world::Activity& act, time::Seconds t,
                                     std::int64_t extra_water = 0) const;
    [[nodiscard]] static std::vector<num::Point> route(const world::World& w, ecs::Id camp, num::Point from,
                                                       num::Point to);
    [[nodiscard]] static bool visible(const world::World& w, ecs::Id camp, num::Point from, num::Point to);
    [[nodiscard]] const LivingRules& rules() const { return rules_; }

private:
    friend class FireRules;
    friend class Crafting;
    friend class Learning;
    [[nodiscard]] num::Point use_spot(const world::World& w, world::Beings::Handle h, ecs::Id camp,
                                      std::size_t need) const;
    void thermal_alarm(world::Context& c, world::Beings::Handle h);
    void notice(world::Context& c, world::Beings::Handle h, ecs::Id camp);
    void choose(world::Context& c, world::Beings::Handle h, ecs::Id camp);
    void continue_goal(world::Context& c, world::Beings::Handle h, ecs::Id camp);
    void settle(world::Context& c, world::Beings::Handle h, ecs::Id camp, bool interrupted);
    void begin(world::Context& c, world::Beings::Handle h, world::LivingAct what, time::Seconds takes, num::Point to);
    void renew(world::Context& c, world::Beings::Handle h);
    void sleep_dream(world::Context& c, world::Beings::Handle h, ecs::Id camp);
    void place_dream(world::Context& c, world::Beings::Handle h, std::int64_t subject, num::Point place);
    void dream_consequence(world::Context& c, world::Beings::Handle h, ecs::Id camp, bool arrived);
    const LivingRules& rules_;
    std::array<NeedUse, 3> uses_{};
};
}  // namespace kd::demo
