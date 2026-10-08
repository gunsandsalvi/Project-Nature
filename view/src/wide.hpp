// T2.9a.1: owned real-object aggregates and canonical look-only ground demand (PRE-28, WLD-01/WLD-13).
#pragma once
#include <array>
#include <string>
#include "projection.hpp"
namespace kd::view {
struct WideRecord {
    std::string key;
    std::uint64_t id = 0, camp_id = 0;
    std::int64_t camp_index = -1;
    double east = 0, north = 0;
    std::uint32_t appearance = 0;
    std::uint8_t activity = 0, facing = 0;
    double phase = 0;
    std::size_t count = 0;
    std::vector<std::uint64_t> members;
};
struct WideFrame {
    std::vector<WideRecord> records;
    std::size_t record_count = 0, member_count = 0, population_count = 0;
    bool truncated = false;
};
struct GroundTile {
    std::string key;
    int power = 0;
    std::int64_t x = 0, y = 0;
    num::Point west_south, east_north;
    Footprint local;
    std::array<std::string, 4> neighbours;     // west, south, east, north
    std::array<std::uint64_t, 4> border_keys;  // SW, SE, NE, NW; absolute wrapped corner samples
    int variant = 0;
    bool accent = false;
    std::vector<num::Point> details;
    std::vector<std::string> detail_keys;
    std::string stamp_key;
};
WideFrame wide_records(const std::vector<DrawRecord>& records, const std::vector<CampRecord>& camps,
                       const num::Torus& torus, num::Point origin, const std::string& form,
                       const Footprint* bounds = nullptr);
GroundTile canonical_tile(const num::Torus& torus, std::int64_t east, std::int64_t north, int power,
                          std::uint64_t seed);
std::vector<GroundTile> ground_tiles(const num::Torus& torus, num::Point origin, Footprint bounds, int power,
                                     std::uint64_t seed);
}  // namespace kd::view
