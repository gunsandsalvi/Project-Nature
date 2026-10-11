#include "doctest.h"
#include "kd/data/folder.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/save/snapshot.hpp"

TEST_CASE("owned item index skips inert spent stock and follows timed food transfers births and removal") {
    kd::data::Catalogue cat;
    REQUIRE(cat.load(kd::data::read_catalogue(KD_REPO "/data")).empty());
    kd::demo::CrowdWorld camp(333, cat, 1, true, true);
    auto& w = camp.world();
    // This scalar index fixture deliberately mutates spent entries later. Archive
    // conservation/publication is tested separately with packing enabled.
    w.set_archive_enabled(false);
    std::vector<kd::ecs::Id> people;
    w.beings().each([&](kd::ecs::Id id, auto h) {
        if (w.beings().raw().all_of<kd::world::Person>(h)) people.push_back(id);
    });
    REQUIRE(people.size() >= 2);
    const auto root = cat.find("item", "base:roots"), meat = cat.find("item", "base:meat"),
               kernels = cat.find("item", "base:kernels");
    REQUIRE(root);
    REQUIRE(meat);
    REQUIRE(kernels);
    if (!root || !meat || !kernels) return;
    const auto home = camp.camp_ids().front();
    const auto make = [&](std::uint32_t kind, kd::ecs::Id owner, bool live = false) {
        const auto h = w.make_thing();
        w.things().raw().emplace<kd::world::Place>(h);
        auto& item = w.things().raw().emplace<kd::world::Item>(h);
        item.kind = item.material = kind;
        item.home = home;
        item.owner = owner;
        item.mass = live ? 1000000 : 0;
        item.state = live ? 0 : 4;
        return w.things().id_of(h);
    };
    for (int n = 0; n < 500; ++n) (void)make(*kernels, people[0]);
    const auto id = make(*root, people[0], true);
    (void)make(*root, people[1], true);
    const auto before = kd::save::write_snapshot(w.save());
    const auto& view = std::as_const(w);
    CHECK(view.items_owned(people[0], *root).size() == 1);
    CHECK(view.items_owned(people[0], *kernels).empty());
    CHECK(view.items_owned(people[0], *meat).empty());
    CHECK(kd::save::write_snapshot(w.save()) == before);
    struct Mutations : kd::world::System {
        kd::ecs::Id first{}, second{};
        std::uint32_t roots = 0, meat = 0;
        std::string_view name() const override { return "owned item index mutations"; }
        void handle(kd::world::Context&, const kd::event::Event&) override {}
        void command(kd::world::Context& c, const kd::world::Command& cmd) override {
            auto& w = c.world();
            const auto& view = std::as_const(w);
            const kd::ecs::Id id{cmd.a};
            CHECK(view.items_owned(first, roots).size() == 1);
            auto& item = w.things().raw().get<kd::world::Item>(w.things().handle(id));
            item.owner = second;
            item.kind = meat;
            c.item_changed(id);
            CHECK(view.items_owned(first, roots).empty());
            CHECK(view.items_owned(second, meat).size() == 1);
            CHECK(view.items_owned(second, meat).front().id == id);
            item.mass = 0;
            item.state = 4;
            c.item_changed(id);
            CHECK(view.items_owned(second, meat).empty());
            // The reference index still carries a spent entry's retained timer.
            auto& timer = w.things().raw().emplace<kd::world::HeatTimer>(w.things().handle(id));
            timer.item = timer.chance_source = id;
            timer.completed = 1;
            c.item_changed(id);
            CHECK(view.items_owned(second, meat).size() == 1);
            const auto born = w.make_thing();
            const auto new_id = w.things().id_of(born);
            w.things().raw().emplace<kd::world::Place>(born);
            w.things().raw().emplace<kd::world::Item>(born, item);
            c.item_changed(new_id);
            CHECK(view.items_owned(second, meat).size() == 1);
            auto& live = w.things().raw().get<kd::world::Item>(born);
            live.mass = 1000000;
            live.state = 0;
            c.item_changed(new_id);
            CHECK(view.items_owned(second, meat).size() == 2);
            CHECK(view.items_owned(second, meat)[0].id < view.items_owned(second, meat)[1].id);
            w.things().end(new_id);
            c.item_changed(new_id);
            CHECK(view.items_owned(second, meat).size() == 1);
        }
    } mutations;
    mutations.first = people[0];
    mutations.second = people[1];
    mutations.roots = *root;
    mutations.meat = *meat;
    w.set_command_taker(mutations);
    (void)w.command(w.frontier(), 906, id.value, 0);
    w.run_to(w.frontier() + 1);
    // Outside events mutable access invalidates both indexes, including handle changes from shuffled storage.
    w.things().fuzz(16);
    CHECK(view.items_owned(people[1], *meat).size() == 1);
    CHECK(view.things().id_of(view.items_owned(people[1], *meat).front().handle) == id);
    auto& changed = w.things().raw().get<kd::world::Item>(w.things().handle(id));
    changed.owner = people[0];
    CHECK(view.items_owned(people[1], *meat).empty());
    CHECK(view.items_owned(people[0], *meat).size() == 1);
}

TEST_CASE("moving an active item keeps old empty locations bounded in the chooser index") {
    kd::data::Catalogue catalogue;
    REQUIRE(catalogue.load(kd::data::read_catalogue(KD_REPO "/data")).empty());
    kd::demo::CrowdWorld camp(9004, catalogue, 1, true, true, true);
    auto& w = camp.world();
    const auto home = camp.camp_ids().front();
    kd::ecs::Id moved{};
    w.things().each([&](auto id, auto h) {
        if (!moved.value && !w.things().raw().template all_of<kd::world::Fire>(h)) moved = id;
    });
    REQUIRE(moved.value != 0);
    if (!moved.value) return;
    const auto original_sites = w.item_sites().size();
    struct Moves final : kd::world::System {
        kd::ecs::Id item{}, home{};
        std::string_view name() const override { return "declared active site migration"; }
        void handle(kd::world::Context&, const kd::event::Event&) override {}
        void command(kd::world::Context& c, const kd::world::Command& command) override {
            auto& w = c.world();
            const auto centre = w.beings().raw().get<kd::world::Place>(w.beings().handle(home)).at;
            w.things().raw().get<kd::world::Place>(w.things().handle(item)).at =
                w.torus().moved(centre, {static_cast<std::int64_t>(command.a), 2500});
            c.item_changed(item);
        }
    } moves;
    moves.item = moved;
    moves.home = home;
    w.set_command_taker(moves);
    for (std::uint64_t step = 0; step < 100; ++step) {
        (void)w.command(w.frontier(), 950, step, 0);
        w.run_to(w.frontier() + 1);
        const auto before = kd::save::write_snapshot(w.save());
        const auto& sites = w.item_sites();
        const auto empty = static_cast<std::size_t>(
            std::count_if(sites.begin(), sites.end(), [](const auto& site) { return site.items.empty(); }));
        CHECK(sites.size() <= original_sites + 16);
        CHECK((empty < 16 || empty * 4 < sites.size()));
        CHECK(kd::save::write_snapshot(w.save()) == before);
        std::size_t copies = 0;
        for (const auto& site : sites)
            copies += std::count_if(site.items.begin(), site.items.end(),
                                    [&](const auto& entry) { return entry.id == moved; });
        CHECK(copies == 1);
    }
}
