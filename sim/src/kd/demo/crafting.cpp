#include "kd/demo/crafting.hpp"
#include "kd/chance/chance.hpp"
#include "kd/demo/discovery.hpp"
#include "kd/demo/discovery_scene.hpp"
namespace kd::demo {
namespace {
void stock(world::World& w, ecs::Id home, std::uint32_t kind, std::int64_t mass, num::Point at) {
    const auto& definition = w.catalogue().kind<data::ItemKind>()[kind];
    while (mass > 0) {
        const auto h = w.make_thing();
        w.things().raw().emplace<world::Place>(h, at);
        auto& item = w.things().raw().emplace<world::Item>(h);
        item.home = home;
        item.kind = kind;
        item.material = kind;
        item.mass = std::min(mass, definition.mass);
        item.length = definition.length;
        mass -= item.mass;
    }
}
}  // namespace
void Crafting::initialise(world::World& w) {
    auto& raw = w.beings().raw();
    // Canonical IDs fix both the material order and each final partial portion.
    w.beings().each([&](ecs::Id id, world::Beings::Handle h) {
        if (const auto* person = raw.try_get<world::Person>(h)) {
            KD_CHECK((!raw.any_of<world::Knowledge, world::Work>(h)), "Fresh craft scene is initialised once");
            raw.emplace<world::Work>(h);
            auto& know = raw.emplace<world::Knowledge>(h);
            know.settled = w.frontier();
            const chance::Draws draws(w.seed(), chance::name("founder traits"), id.value, 0,
                                      chance::name("curiosity and kindness"));
            know.curiosity = static_cast<std::uint8_t>(draws.between(0, 20, 80));
            know.kindness = static_cast<std::uint8_t>(draws.between(1, 20, 80));
            const auto age_extra = static_cast<std::int64_t>((person->age_years - 18) / 10) * 100;
            know.sectors[5] = {3000 + age_extra, 3000 + age_extra, 0, -1};
            know.sectors[4] = {2000 + age_extra, 2000 + age_extra, 0, -1};
            know.sectors[2] = {1000 + age_extra, 1000 + age_extra, 0, -1};
            const auto& recipes = w.catalogue().kind<data::Blueprint>();
            for (std::uint32_t r = 0; r < recipes.size(); ++r) {
                if (!recipes[r].starting) continue;
                world::Skill skill;
                skill.recipe = r;
                skill.known = 1;
                skill.practice = {3000 + age_extra, 3000 + age_extra, 0, -1};
                know.skills.push_back(skill);
            }
            Discovery::starting(w, h);
        } else if (auto* camp = raw.try_get<world::Camp>(h)) {
            KD_CHECK(!raw.all_of<world::CraftHistory>(h), "Fresh stock is materialised once");
            raw.emplace<world::CraftHistory>(h);
            raw.emplace<world::Lessons>(h);
            const auto stone = camp->stone_mg, wood = camp->wood_mg;
            const auto& scenes = w.catalogue().kind<DiscoveryScene>();
            KD_CHECK(scenes.size() == 1, "Fresh Discovery camp needs its recorded scene");
            std::array<std::int64_t, 3> remaining{0, stone, wood}, shares{};
            for (const auto& input : scenes[0].stock) {
                const auto aggregate = static_cast<std::size_t>(input.aggregate);
                const auto source = aggregate == 1 ? stone : wood;
                auto amount = input.mass;
                if (aggregate != 0) {
                    shares[aggregate] += input.share;
                    amount = shares[aggregate] == 1000000 ? remaining[aggregate] : source * input.share / 1000000;
                    remaining[aggregate] -= amount;
                }
                const std::array<num::Point, 3> places{camp->food_at, camp->stone_at, camp->wood_at};
                stock(w, id, input.kind.index, amount, places[static_cast<std::size_t>(input.site)]);
            }
            KD_CHECK(remaining[1] == 0 && remaining[2] == 0, "Scene shares conserve aggregate stocks");
            camp->stone_mg = 0;
            camp->wood_mg = 0;
        }
    });
}
std::array<std::int64_t, 18> Crafting::characteristics(const data::Catalogue& c, const world::Item& item) {
    const auto& kind = c.kind<data::ItemKind>()[item.kind];
    auto out = c.kind<data::ItemKind>()[item.material].characteristics;
    if (!kind.inherit) out = kind.characteristics;
    for (const auto& change : kind.state_changes)
        if (change.state == item.state) out[static_cast<std::size_t>(change.characteristic)] = change.value;
    for (std::size_t i = 0; i < 18; ++i)
        if (item.changed_mask & (1U << i)) out[i] = item.changed[i];
    if (item.made_at >= 0 && kind.primary != 8 && kind.primary != 9) {
        const auto primary = static_cast<std::size_t>(kind.primary);
        const auto quality = item.quality <= 1 ? -1 : item.quality >= 4 ? 1 : 0;
        out[primary] = std::clamp<std::int64_t>(out[primary] + quality - item.wear / 1000000, 0, 5);
    }
    return out;
}
data::FitInput Crafting::physical(const data::Catalogue& c, const world::Item& item) {
    const auto& kind = c.kind<data::ItemKind>()[item.kind];
    const auto& material = c.kind<data::ItemKind>()[item.material];
    return {kind.inherit ? std::string_view(material.material_class) : std::string_view(kind.material_class), kind.form,
            characteristics(c, item), item.length, item.mass};
}
std::int64_t Crafting::available(const world::World& w, ecs::Id item, ecs::Id self) {
    const auto h = w.things().find(item);
    if (!h || !w.things().raw().all_of<world::Item>(*h)) return 0;
    auto left = w.things().raw().get<world::Item>(*h).mass;
    w.beings().each([&](ecs::Id id, world::Beings::Handle person) {
        if (id == self) return;
        const auto* life = w.beings().raw().try_get<world::Life>(person);
        if (life && life->meal_item == item) left -= life->carried_food;
        const auto* work = w.beings().raw().try_get<world::Work>(person);
        if (!work) return;
        for (const auto& r : work->inputs) {
            if (r.item != item) continue;
            if (r.retained)
                left = 0;
            else
                left -= r.mass;
        }
    });
    return std::max<std::int64_t>(0, left);
}
bool Crafting::tool_free(const world::World& w, ecs::Id item, ecs::Id self) {
    bool free = true;
    w.beings().each([&](ecs::Id id, world::Beings::Handle person) {
        if (id == self) return;
        const auto* life = w.beings().raw().try_get<world::Life>(person);
        if (life && life->meal_item == item) free = false;
        const auto* work = w.beings().raw().try_get<world::Work>(person);
        if (!work) return;
        for (const auto& r : work->inputs)
            if (r.item == item) free = false;
    });
    return free;
}
std::int64_t Crafting::total(const world::World& w, ecs::Id camp, std::string_view material_class) {
    std::int64_t mass = 0;
    w.things().each([&](ecs::Id, world::Things::Handle h) {
        const auto* item = w.things().raw().try_get<world::Item>(h);
        if (!item || item->home != camp) return;
        if (physical(w.catalogue(), *item).material_class == material_class) mass += item->mass;
    });
    return mass;
}
}  // namespace kd::demo
