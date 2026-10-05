// The world's entities (A3.2): EnTT 4.0.0 behind this thin layer, so no rule creates or destroys entities itself and a
// later change of library stays here. Each registry knows its components, in the order of their names: their pools
// are made in that order at start, so a new world and a reopened one have the same pools in the same order.
// EnTT's own order is not canonical, so whatever decides walks the entities by id (each()), never a pool's order;
// the order fuzzer (fuzz()) scrambles every pool, and no result may move.
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include <entt.hpp>

#include "kd/chance/chance.hpp"
#include "kd/core/check.hpp"
#include "kd/ecs/component.hpp"
#include "kd/ecs/id.hpp"
#include "kd/num/digest.hpp"

namespace kd::ecs {

/// Every entity's id, its one component in every registry.
struct Ident {
    static constexpr std::string_view name = "ident";
    static constexpr std::uint32_t version = 1;
    Id id;

    template <typename V, typename Self>
    static void visit(V& v, Self& c) {
        v.id({"id", "the entity's id, never reused"}, c.id);
    }
};

/// The 64-bit handles of the things' registry, room for more than a million at once (A3.2).
enum class Handle64 : std::uint64_t {};

/// Implements RES-05, see A3.2: from id to handle, the ids in order. Within a family ids only grow, so each new one
/// goes at the end of its family's list; an ended entity leaves a gap, swept out once its family's gaps pass a quarter
/// of its live entries.
template <typename Handle>
class IdMap {
public:
    void add(Id id, Handle h) {
        Family& f = families_[static_cast<std::size_t>(id.family())];
        KD_CHECK(f.entries.empty() || f.entries.back().id < id, "ecs::IdMap: a family's ids come in order");
        f.entries.push_back({id, h, true});
        ++f.live;
        ++live_;
    }

    [[nodiscard]] std::optional<Handle> find(Id id) const {
        const Family& f = families_[static_cast<std::size_t>(id.family())];
        const std::size_t i = index(f, id);
        if (i == f.entries.size() || !f.entries[i].live) {
            return std::nullopt;
        }
        return f.entries[i].handle;
    }

    void remove(Id id) {
        Family& f = families_[static_cast<std::size_t>(id.family())];
        const std::size_t i = index(f, id);
        KD_CHECK(i != f.entries.size() && f.entries[i].live, "ecs::IdMap: no live entity with that id");
        f.entries[i].live = false;
        --f.live;
        --live_;
        if ((f.entries.size() - f.live) * 4 > f.live) {
            std::erase_if(f.entries, [](const Entry& e) { return !e.live; });
        }
    }

    /// Each live entity, (id, handle), in the order of their ids.
    template <typename F>
    void each(F fn) const {
        for (const Family& f : families_) {
            for (const Entry& e : f.entries) {
                if (e.live) {
                    fn(e.id, e.handle);
                }
            }
        }
    }

    [[nodiscard]] std::size_t size() const { return live_; }

private:
    struct Entry {
        Id id;
        Handle handle;
        bool live;
    };
    struct Family {
        std::vector<Entry> entries;
        std::size_t live = 0;
    };

    [[nodiscard]] static std::size_t index(const Family& f, Id id) {
        const auto at = std::lower_bound(f.entries.begin(), f.entries.end(), id,
                                         [](const Entry& e, Id wanted) { return e.id < wanted; });
        return at != f.entries.end() && at->id == id ? static_cast<std::size_t>(at - f.entries.begin())
                                                     : f.entries.size();
    }

    std::array<Family, 16> families_;
    std::size_t live_ = 0;
};

namespace detail {

template <typename... Cs>
constexpr bool names_in_order() {
    const std::string_view names[] = {Cs::name...};
    for (std::size_t i = 1; i < sizeof...(Cs); ++i) {
        if (!(names[i - 1] < names[i])) {
            return false;
        }
    }
    return true;
}

}  // namespace detail

/// Implements RES-05 and MAT-13, see A3.2: one registry of entities and the components it may hold, listed in the
/// order of their names.
template <typename HandleT, typename... Components>
class Registry {
    static_assert(detail::names_in_order<Components...>(), "a registry's components are listed in name order");

public:
    using Handle = HandleT;
    using Raw = entt::basic_registry<Handle>;

    Registry() { (raw_.template storage<Components>(), ...); }

    /// A new entity with its id, which must be newer than every id this registry has seen.
    Handle make(Id id) {
        const Handle h = raw_.create();
        raw_.template emplace<Ident>(h, id);
        ids_.add(id, h);
        return h;
    }

    /// Ends an entity: it leaves the registry, and its id is never used again.
    void end(Id id) {
        const std::optional<Handle> h = ids_.find(id);
        KD_CHECK(h.has_value(), "ecs::Registry: no entity with that id to end");
        raw_.destroy(*h);
        ids_.remove(id);
    }

    [[nodiscard]] std::optional<Handle> find(Id id) const { return ids_.find(id); }
    [[nodiscard]] Id id_of(Handle h) const { return raw_.template get<Ident>(h).id; }
    [[nodiscard]] std::size_t size() const { return ids_.size(); }

    /// EnTT's registry, for a system's own reads and writes; never for an order that decides.
    [[nodiscard]] Raw& raw() { return raw_; }
    [[nodiscard]] const Raw& raw() const { return raw_; }

    /// Each entity, (id, handle), in the order of their ids.
    template <typename F>
    void each(F f) const {
        ids_.each(f);
    }

    /// Every component of every entity, entities in id order and components in name order. Implements RES-05.
    void digest(num::Digest& d) const {
        d.u64(ids_.size());
        ids_.each([&](Id id, Handle h) {
            d.u64(id.value);
            (digest_one<Components>(h, d), ...);
        });
    }

    /// Scrambles every pool's order by a key, as the order fuzzer does before each batch (A3.2).
    void fuzz(std::uint64_t key) { (scramble<Components>(key), ...); }

private:
    template <typename C>
    void digest_one(Handle h, num::Digest& d) const {
        if (const C* c = raw_.template try_get<C>(h)) {
            d.u8(1);
            digest_component(*c, d);
        } else {
            d.u8(0);
        }
    }

    template <typename C>
    void scramble(std::uint64_t key) {
        const chance::Draws draws(key, chance::name("order fuzzer"), 0, 0, chance::name(C::name));
        raw_.template sort<C>([&](Handle a, Handle b) {
            const std::uint64_t x = draws.bits(id_of(a).value);
            const std::uint64_t y = draws.bits(id_of(b).value);
            return x != y ? x < y : id_of(a) < id_of(b);
        });
    }

    Raw raw_;
    IdMap<Handle> ids_;
};

}  // namespace kd::ecs
