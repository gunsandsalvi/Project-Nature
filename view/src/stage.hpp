// The stage (A4.6): the view's own copy of what is drawn, filled by a feed of changes, so a family uploads a still
// copy once and touches it again only when it changes. Each copy belongs to a family (the ground, cover, plants,
// things, figures and the rest) and is one of its forms, placed in whole centimetres east and north of the world's
// centre and up from its datum. It touches no Godot, so its tests run alone; the families read its changes and
// draw through Godot's RenderingServer.
#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <vector>

namespace kd::view {

/// One copy on the stage.
struct Copy {
    std::uint32_t family = 0;
    std::uint32_t form = 0;
    std::int64_t east = 0;  // centimetres
    std::int64_t north = 0;
    std::int64_t up = 0;
    double turn = 0.0;  // degrees clockwise from north
    double scale = 1.0;
    std::array<float, 8> look{};  // the family's own numbers for it: its layer, tint, wear, seed and the rest (A4.6)

    bool operator==(const Copy&) const = default;
};

/// What changed on the stage: a copy put there or changed, or a copy gone.
struct Change {
    std::uint64_t id = 0;
    bool gone = false;
};

/// Implements PRE-22, see A4.6: the copies the view draws, and the changes since each family last read them.
class Stage {
public:
    /// Puts a copy on the stage, or changes it; a copy put again unchanged is no change.
    void put(std::uint64_t id, const Copy& copy);
    /// Takes a copy off the stage; a copy not there is no change.
    void drop(std::uint64_t id);

    /// A copy by its number, or nothing.
    [[nodiscard]] const Copy* find(std::uint64_t id) const;
    [[nodiscard]] std::size_t size() const { return copies_.size(); }
    [[nodiscard]] const std::map<std::uint64_t, Copy>& copies() const { return copies_; }

    /// The changes since the last call, each copy once, in the order of their numbers.
    std::vector<Change> drain();

private:
    std::map<std::uint64_t, Copy> copies_;
    std::map<std::uint64_t, bool> changed_;  // by copy, whether it is gone
};

}  // namespace kd::view
