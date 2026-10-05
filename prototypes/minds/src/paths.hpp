// P6's paths (A11, research 10): in levels, as Dwarf Fortress, HPA* and Songs of Syx do. Connected regions reject an
// impossible trip at once. The land's 32 x 32 blocks are clusters, each split into the parts of it joined inside it;
// the parts meet at entrances on the clusters' borders, and from each entrance a field of distances over its part is
// made once at the start (A11's flow fields inside clusters). A trip between two parts follows the path between them
// over the entrances, found by A* once and cached by the pair, plus the two fields' distances at its ends; a trip
// inside one part is a small A*. Every search breaks ties by cell or entrance, and a cached path is found from the two
// parts' own middles, never from the trip's ends, so a cached path and a fresh one are the same and the cache changes
// only the time. Pre-production code (research 00).
#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include "land.hpp"

namespace minds {

// A trip: its length in metres of open ground (rough ground counting more), and the cells where it passes from
// cluster to cluster, for a view to walk it along.
struct Trip {
    float metres = 0.0F;
    std::vector<std::int32_t> via;
};

// How trips were found: inside one part, by a cached path, by a path found and cached, or not at all.
struct TripCounts {
    std::int64_t inside = 0;
    std::int64_t cached = 0;
    std::int64_t fresh = 0;
    std::int64_t none = 0;
};

// The path between two parts over the entrances: whether one exists, its entrances, and its length between the
// first and the last.
struct PartPath {
    bool found = false;
    float metres = 0.0F;
    std::vector<std::int32_t> via;
};

// One thread's working space: the searches' arrays, and the paths it found since the last merge into the shared
// cache, which no other thread touches.
class PathScratch {
public:
    // For trips over a land of this many entrances (Paths::entrances), or none for searches inside one cluster.
    explicit PathScratch(int entrances = 0);
    TripCounts counts;

private:
    friend class Paths;
    static constexpr int kInCluster = kBlock * kBlock;
    std::array<float, kInCluster> cost_{};
    std::array<std::uint32_t, kInCluster> seen_{};    // the search that last reached a cell
    std::array<std::uint32_t, kInCluster> closed_{};  // the search that last expanded it
    std::uint32_t search_ = 0;
    std::vector<std::pair<float, std::int32_t>> open_;
    std::unordered_map<std::uint64_t, PartPath> fresh_;
    std::vector<float> node_cost_;
    std::vector<std::int32_t> node_came_;
    std::vector<std::uint32_t> node_seen_;
    std::vector<std::uint32_t> node_closed_;
    std::vector<float> goal_cost_;
    std::vector<std::uint32_t> goal_seen_;
};

class Paths {
public:
    explicit Paths(const Land& land);

    [[nodiscard]] std::int32_t region(int cell) const { return region_[static_cast<std::size_t>(cell)]; }
    // A trip between two cells; false when none exists. Reads the shared cache; what it finds goes to the scratch.
    bool trip(int from, int to, PathScratch* s, Trip* out) const;
    // The paths a thread found, into the shared cache: between batches of trips, with no trip running.
    void merge(PathScratch* s);
    [[nodiscard]] int entrances() const { return static_cast<int>(node_cell_.size()); }
    [[nodiscard]] int parts() const { return static_cast<int>(part_middle_.size()); }
    [[nodiscard]] std::size_t cached() const { return cache_.size(); }

private:
    struct Edge {
        std::int32_t to;
        float metres;
    };
    using Reached = std::vector<std::pair<std::int32_t, float>>;

    // A* from one cell to another of the same part: its length, or a negative number when none.
    float search(int from, int to, PathScratch* s) const;
    // Dijkstra's flood over a cell's part, into a field of distances by the cell's place in its cluster.
    void flood(int cell, PathScratch* s, std::vector<float>* field) const;
    // A* over the entrances, from the start entrances at their costs to the goal ones, each with its cost on.
    bool abstract(const Reached& starts, const Reached& goals, int goal_cell, PathScratch* s, PartPath* out) const;
    // An entrance reached at a cost, kept if it is the cheapest way there yet.
    static void relax(PathScratch* s, std::uint32_t id, std::int32_t from, std::int32_t to, float cost, float h);
    [[nodiscard]] bool step_ok(int from, int dir) const;
    // An entrance's distance to a cell of its part.
    [[nodiscard]] float field(std::int32_t node, int cell) const;

    const Land& land_;
    std::vector<std::int32_t> region_;
    std::vector<std::int32_t> part_;         // each cell's part, -1 where no one stands
    std::vector<std::int32_t> part_middle_;  // each part's cell nearest its cluster's middle
    std::vector<std::vector<std::int32_t>> part_nodes_;
    std::vector<std::int32_t> node_cell_;
    std::vector<std::vector<Edge>> edges_;
    std::vector<std::uint16_t> fields_;  // each entrance's distances over its cluster, in tenths of a metre
    std::unordered_map<std::uint64_t, PartPath> cache_;
};

}  // namespace minds
