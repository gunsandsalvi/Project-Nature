// Conserved camp things and generic work (MAT-01, MAT-04, MAT-09, TIM-17).
#pragma once
#include "kd/data/craft.hpp"
#include "kd/world/world.hpp"
namespace kd::demo {
class Living;
class ChoiceSet;
struct KnownUse {
    std::uint32_t recipe = 0;
    std::uint8_t role = 0, need = 3;
    std::int64_t benefit = 0, score = 0;
};
class Crafting {
public:
    // Only fresh labelled Discovery scenes call this; never an opening/conversion hook.
    static void initialise(world::World& w, bool fire_already_out = false);
    // Called only at bodily decision/event boundaries, never by rendering.
    static bool prepare_lesson(world::Context& c, world::Beings::Handle teacher, world::Beings::Handle learner,
                               std::uint32_t recipe, std::uint64_t session, Living* living = nullptr);
    [[nodiscard]] static bool lesson_reachable(world::Context& c, world::Beings::Handle teacher,
                                               world::Beings::Handle learner, std::uint32_t recipe);
    static bool choose(Living& living, world::Context& c, world::Beings::Handle h, ChoiceSet* proposals = nullptr);
    static bool continue_work(Living& living, world::Context& c, world::Beings::Handle h);
    static void settle(Living& living, world::Context& c, world::Beings::Handle h, bool interrupted, bool try_event);
    static bool meal(Living& living, world::Context& c, world::Beings::Handle h);
    static void settle_meal(world::Context& c, world::Beings::Handle h, std::int64_t eaten, bool finished);
    // Pure fixed-point rules shared by resolution and calibration tests.
    [[nodiscard]] static std::int64_t success(const world::World& w, world::Beings::Handle h, std::uint32_t recipe,
                                              std::span<const ecs::Id> roles, time::Seconds at = -1);
    [[nodiscard]] static std::int64_t duration(const data::Blueprint& recipe, std::int64_t edge, bool tool);
    [[nodiscard]] static std::int64_t time_cost(const world::World& w, world::Beings::Handle h, std::int64_t seconds);
    // MAT-22: one useful downstream use, from this mind's recipes and perceived facts.
    // A reusable tool's value is not its weight. No physical characteristics are read.
    [[nodiscard]] static std::vector<KnownUse> known_uses(const data::Catalogue& catalogue,
                                                          const world::Knowledge& mind,
                                                          const std::array<std::int64_t, 3>& needs,
                                                          const world::Item& item, const world::Familiar& familiar,
                                                          std::int64_t warmth = 100);
    static void wear(world::Context& c, ecs::Id tool, std::int64_t worked, std::int64_t rate);

    [[nodiscard]] static std::array<std::int64_t, 18> characteristics(const data::Catalogue& c,
                                                                      const world::Item& item);
    [[nodiscard]] static data::FitInput physical(const data::Catalogue& c, const world::Item& item);
    [[nodiscard]] static std::int64_t available(const world::World& w, ecs::Id item, ecs::Id self = {});
    [[nodiscard]] static bool tool_free(const world::World& w, ecs::Id item, ecs::Id self = {});
    [[nodiscard]] static std::int64_t total(const world::World& w, ecs::Id camp, std::string_view material_class);

private:
    struct Decision;
    static bool start_meal(Living& living, world::Context& c, world::Beings::Handle h, ecs::Id item,
                           std::int64_t portion);
    static bool meal(Living& living, world::Context& c, world::Beings::Handle h, const Decision& decision,
                     ChoiceSet* proposals = nullptr);
};
}  // namespace kd::demo
