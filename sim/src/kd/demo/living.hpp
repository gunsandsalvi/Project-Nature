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
    void start(world::World& w);
    [[nodiscard]] std::int64_t reach() const override { return 10000; }
    void bounds(const world::World& w, time::Seconds a, time::Seconds b, std::span<const ecs::Id> owners,
                std::vector<world::Bound>& out) const override;
    void near(const world::World& w, time::Seconds a, time::Seconds b, const world::Bound& bound,
              std::vector<ecs::Id>& out) const override;
    [[nodiscard]] static std::array<std::int64_t, 3> needs(const world::Life& life);
    [[nodiscard]] world::Life sample(world::Life life, const world::Activity& act, time::Seconds t) const;
    [[nodiscard]] static std::vector<num::Point> route(const world::World& w, ecs::Id camp, num::Point from,
                                                       num::Point to);
    [[nodiscard]] static bool visible(const world::World& w, ecs::Id camp, num::Point from, num::Point to);
    [[nodiscard]] const LivingRules& rules() const { return rules_; }

private:
    [[nodiscard]] num::Point use_spot(const world::World& w, world::Beings::Handle h, ecs::Id camp,
                                      std::size_t need) const;
    void notice(world::Context& c, world::Beings::Handle h, ecs::Id camp);
    void choose(world::Context& c, world::Beings::Handle h, ecs::Id camp);
    void continue_goal(world::Context& c, world::Beings::Handle h, ecs::Id camp);
    void settle(world::Context& c, world::Beings::Handle h, ecs::Id camp, bool interrupted);
    void begin(world::Context& c, world::Beings::Handle h, world::LivingAct what, time::Seconds takes, num::Point to);
    void renew(world::Context& c, world::Beings::Handle h);
    const LivingRules& rules_;
    std::array<NeedUse, 3> uses_{};
};
}  // namespace kd::demo
