#include "wide.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include "kd/num/whole.hpp"
namespace kd::view {
namespace {
constexpr std::size_t kRows = 512, kMembers = 8192;
std::uint64_t mix(std::uint64_t n) {
    n ^= n >> 30;
    n *= 0xbf58476d1ce4e5b9ULL;
    n ^= n >> 27;
    n *= 0x94d049bb133111ebULL;
    return n ^ (n >> 31);
}
std::uint64_t look_key(num::Point p, std::uint64_t seed) {
    return mix(seed ^ mix(static_cast<std::uint64_t>(p.x)) ^
               mix(static_cast<std::uint64_t>(p.y) + 0x9e3779b97f4a7c15ULL));
}
std::string tile_key(int power, std::int64_t x, std::int64_t y) {
    return "ground/" + std::to_string(power) + "/" + std::to_string(x) + "/" + std::to_string(y);
}
std::int64_t pitch(int power) {
    return std::int64_t{100} << std::clamp(power, 0, 22);
}
std::vector<std::int64_t> indices(double low, double high, std::int64_t size, std::int64_t cell) {
    std::vector<std::int64_t> result;
    const auto count = (size + cell - 1) / cell;
    if (high - low >= static_cast<double>(size)) {
        if (count > 256) return result;
        for (std::int64_t i = 0; i < count; ++i) result.push_back(i);
    } else {
        const auto a = static_cast<std::int64_t>(std::floor(low));
        const auto b = static_cast<std::int64_t>(std::floor(high));
        const auto first = num::floor_mod(a, size) / cell, last = num::floor_mod(b, size) / cell;
        const auto crossed =
            std::floor(low / static_cast<double>(size)) != std::floor(high / static_cast<double>(size));
        const auto needed = crossed ? count - first + last + 1 : last - first + 1;
        if (needed > 256) return result;
        if (crossed) {
            for (auto i = first; i < count; ++i) result.push_back(i);
            for (std::int64_t i = 0; i <= last; ++i) result.push_back(i);
        } else
            for (auto i = first; i <= last; ++i) result.push_back(i);
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
    }
    return result;
}
}  // namespace
WideFrame wide_records(const std::vector<DrawRecord>& input, const std::vector<CampRecord>& camps,
                       const num::Torus& torus, num::Point origin, const std::string& form, const Footprint* bounds) {
    WideFrame out;
    out.population_count = input.size();
    if (form != "tiny" && form != "group" && form != "overview-fixture") return out;
    struct Identity {
        std::string key;
        std::uint64_t id;
        num::Point anchor;
        bool fixed;
    };
    const auto identify = [&](const DrawRecord& row) {
        const auto position =
            torus.wrap(origin.x + std::llround(row.east * 100), origin.y + std::llround(row.north * 100));
        Identity identity{"walker/" + std::to_string(row.id), row.id, position, false};
        if (form != "tiny" && row.camp < camps.size()) {
            identity.id = camps[row.camp].id;
            identity.key = "camp/" + std::to_string(identity.id);
            if (form == "group") {
                identity.anchor = {position.x / 25600 * 25600, position.y / 25600 * 25600};
                identity.key +=
                    "/cell/" + std::to_string(position.x / 25600) + "/" + std::to_string(position.y / 25600);
            } else {
                identity.anchor = camps[row.camp].place;
                identity.fixed = true;
            }
        }
        return identity;
    };
    const auto valid = [](const DrawRecord& row) {
        return std::isfinite(row.east) && std::isfinite(row.north) && std::abs(row.east) <= 1e9 &&
               std::abs(row.north) <= 1e9;
    };
    const auto visible = [&](const DrawRecord& row) {
        if (!valid(row)) return false;
        if (!bounds) return true;
        const auto identity = identify(row);
        double west = row.east, south = row.north, east = west, north = south;
        if (!identity.key.starts_with("walker/")) {
            const auto offset = torus.offset(origin, identity.anchor);
            west = static_cast<double>(offset.dx) / 100.;
            south = static_cast<double>(offset.dy) / 100.;
            east = west;
            north = south;
            if (!identity.fixed) {
                east += static_cast<double>(std::min<std::int64_t>(25600, torus.width() - identity.anchor.x)) / 100.;
                north += static_cast<double>(std::min<std::int64_t>(25600, torus.height() - identity.anchor.y)) / 100.;
            }
        }
        return west <= bounds->east && east >= bounds->west && south <= bounds->north && north >= bounds->south;
    };
    // Select the same lowest keys in every source order, without allocating one entry per off-screen input.
    std::set<std::string> selected;
    for (const auto& row : input)
        if (visible(row)) {
            selected.insert(identify(row).key);
            if (selected.size() > kRows) {
                selected.erase(std::prev(selected.end()));
                out.truncated = true;
            }
        }
    struct Accumulator {
        WideRecord record;
        num::Point anchor;
        std::int64_t dx = 0, dy = 0;
        bool fixed = false;
        std::uint64_t first = UINT64_MAX;
    };
    std::map<std::string, Accumulator> groups;
    std::set<std::pair<std::string, std::uint64_t>> members;
    for (const auto& row : input)
        if (visible(row)) {
            ++out.member_count;
            const auto identity = identify(row);
            if (!selected.contains(identity.key)) continue;
            auto& accumulator = groups[identity.key];
            auto& result = accumulator.record;
            if (!result.count) {
                result.key = identity.key;
                result.id = identity.id;
                accumulator.anchor = identity.anchor;
                accumulator.fixed = identity.fixed;
            }
            ++result.count;
            if (row.id < accumulator.first) {
                accumulator.first = row.id;
                result.camp_index = row.camp < camps.size() ? static_cast<std::int64_t>(row.camp) : -1;
                result.camp_id = row.camp < camps.size() ? camps[row.camp].id : 0;
                result.appearance = row.appearance;
                result.activity = row.activity;
                result.facing = row.facing;
                result.phase = row.phase;
                if (form == "tiny" || identity.key.starts_with("walker/")) {
                    result.east = row.east;
                    result.north = row.north;
                }
            }
            const auto position =
                torus.wrap(origin.x + std::llround(row.east * 100), origin.y + std::llround(row.north * 100));
            const auto delta = torus.offset(accumulator.anchor, position);
            const double fraction_east = row.east * 100 - static_cast<double>(std::llround(row.east * 100));
            const double fraction_north = row.north * 100 - static_cast<double>(std::llround(row.north * 100));
            accumulator.dx += std::llround((static_cast<double>(delta.dx) + fraction_east) * 65536);
            accumulator.dy += std::llround((static_cast<double>(delta.dy) + fraction_north) * 65536);
            members.insert({identity.key, row.id});
            if (members.size() > kMembers) {
                members.erase(std::prev(members.end()));
                out.truncated = true;
            }
        }
    // record_count is retained records; overflow is explicit rather than an invented exact distinct total.
    out.record_count = groups.size();
    for (const auto& [key, id] : members) groups.at(key).record.members.push_back(id);
    for (auto& [key, accumulator] : groups) {
        auto& row = accumulator.record;
        if (!key.starts_with("walker/")) {
            const auto offset = torus.offset(origin, accumulator.anchor);
            row.east = static_cast<double>(offset.dx) / 100.;
            row.north = static_cast<double>(offset.dy) / 100.;
            if (!accumulator.fixed) {
                row.east += static_cast<double>(accumulator.dx) / (6553600.0 * static_cast<double>(row.count));
                row.north += static_cast<double>(accumulator.dy) / (6553600.0 * static_cast<double>(row.count));
            }
        }
        out.records.push_back(std::move(row));
    }
    if (form == "tiny")
        std::sort(out.records.begin(), out.records.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
    return out;
}
GroundTile canonical_tile(const num::Torus& torus, std::int64_t east, std::int64_t north, int power,
                          std::uint64_t seed) {
    GroundTile tile;
    tile.power = std::clamp(power, 0, 22);
    const auto size = pitch(tile.power);
    const auto p = torus.wrap(east, north);
    tile.x = p.x / size;
    tile.y = p.y / size;
    tile.key = tile_key(tile.power, tile.x, tile.y);
    tile.west_south = {static_cast<std::int32_t>(tile.x * size), static_cast<std::int32_t>(tile.y * size)};
    tile.east_north = {static_cast<std::int32_t>(std::min<std::int64_t>((tile.x + 1) * size, torus.width())),
                       static_cast<std::int32_t>(std::min<std::int64_t>((tile.y + 1) * size, torus.height()))};
    const auto nx = (torus.width() + size - 1) / size, ny = (torus.height() + size - 1) / size;
    tile.neighbours = {tile_key(tile.power, num::floor_mod(tile.x - 1, nx), tile.y),
                       tile_key(tile.power, tile.x, num::floor_mod(tile.y - 1, ny)),
                       tile_key(tile.power, (tile.x + 1) % nx, tile.y),
                       tile_key(tile.power, tile.x, (tile.y + 1) % ny)};
    const std::array<num::Point, 4> corners{tile.west_south,
                                            {tile.east_north.x, tile.west_south.y},
                                            tile.east_north,
                                            {tile.west_south.x, tile.east_north.y}};
    for (std::size_t i = 0; i < 4; ++i) tile.border_keys[i] = look_key(torus.wrap(corners[i].x, corners[i].y), seed);
    const auto hash = look_key(tile.west_south, seed);
    tile.variant = static_cast<int>(hash % 3);
    const num::Point stamp_origin{tile.west_south.x / 25600 * 25600, tile.west_south.y / 25600 * 25600};
    tile.accent = (look_key(stamp_origin, seed) >> 8) % 32 == 0;
    tile.stamp_key =
        "stamp/8/" + std::to_string(tile.west_south.x / 25600) + "/" + std::to_string(tile.west_south.y / 25600);
    const auto detail_power = 4;
    const auto detail_size = pitch(detail_power);
    const num::Point detail_origin{tile.west_south.x / 1600 * 1600, tile.west_south.y / 1600 * 1600};
    const auto detail_hash = look_key(detail_origin, seed);
    const auto detail_width = std::min<std::int64_t>(detail_size, torus.width() - detail_origin.x);
    const auto detail_height = std::min<std::int64_t>(detail_size, torus.height() - detail_origin.y);
    for (std::uint64_t i = 0; i < 3; ++i) {
        tile.detail_keys.push_back("detail/" + std::to_string(detail_power) + "/" +
                                   std::to_string(detail_origin.x / detail_size) + "/" +
                                   std::to_string(detail_origin.y / detail_size) + "/" + std::to_string(i));
        const auto h = mix(detail_hash + i + 1);
        tile.details.push_back(
            {static_cast<std::int32_t>(detail_origin.x + h % static_cast<std::uint64_t>(detail_width)),
             static_cast<std::int32_t>(detail_origin.y + (h >> 32) % static_cast<std::uint64_t>(detail_height))});
    }
    return tile;
}
std::vector<GroundTile> ground_tiles(const num::Torus& torus, num::Point origin, Footprint bounds, int power,
                                     std::uint64_t seed) {
    std::vector<GroundTile> result;
    if (!std::isfinite(bounds.west) || !std::isfinite(bounds.east) || !std::isfinite(bounds.south) ||
        !std::isfinite(bounds.north) || bounds.west > bounds.east || bounds.south > bounds.north ||
        std::max({std::abs(bounds.west), std::abs(bounds.east), std::abs(bounds.south), std::abs(bounds.north)}) > 1e12)
        return result;
    const double centre_east = (bounds.west + bounds.east) / 2, centre_north = (bounds.south + bounds.north) / 2;
    const auto centre =
        torus.wrap(origin.x + std::llround(centre_east * 100), origin.y + std::llround(centre_north * 100));
    const double world_width = torus.width() / 100., world_height = torus.height() / 100.;
    const auto local = [&](GroundTile& tile) {
        const auto middle = torus.wrap((std::int64_t{tile.west_south.x} + tile.east_north.x) / 2,
                                       (std::int64_t{tile.west_south.y} + tile.east_north.y) / 2);
        const auto offset = torus.offset(centre, middle);
        const double west = static_cast<double>(std::llround(centre_east * 100)) / 100. +
                            static_cast<double>(offset.dx) / 100. - (middle.x - tile.west_south.x) / 100.;
        const double south = static_cast<double>(std::llround(centre_north * 100)) / 100. +
                             static_cast<double>(offset.dy) / 100. - (middle.y - tile.west_south.y) / 100.;
        tile.local = {west, south, west + (tile.east_north.x - tile.west_south.x) / 100.,
                      south + (tile.east_north.y - tile.west_south.y) / 100.};
    };
    const auto repeats = [](double low, double high, double west, double east, double span) {
        return std::pair<std::int64_t, std::int64_t>{static_cast<std::int64_t>(std::ceil((low - east) / span)),
                                                     static_cast<std::int64_t>(std::floor((high - west) / span))};
    };
    for (power = std::clamp(power, 0, 22); power <= 22; ++power) {
        const auto xs =
            indices(origin.x + bounds.west * 100, origin.x + bounds.east * 100, torus.width(), pitch(power));
        const auto ys =
            indices(origin.y + bounds.south * 100, origin.y + bounds.north * 100, torus.height(), pitch(power));
        if (xs.empty() || ys.empty() || xs.size() * ys.size() > 256) continue;
        std::vector<GroundTile> canonical;
        std::size_t total = 0;
        for (const auto x : xs)
            for (const auto y : ys) {
                auto tile = canonical_tile(torus, x * pitch(power), y * pitch(power), power, seed);
                local(tile);
                const auto [x0, x1] = repeats(bounds.west, bounds.east, tile.local.west, tile.local.east, world_width);
                const auto [y0, y1] =
                    repeats(bounds.south, bounds.north, tile.local.south, tile.local.north, world_height);
                const auto nx = std::max<std::int64_t>(0, x1 - x0 + 1), ny = std::max<std::int64_t>(0, y1 - y0 + 1);
                if (nx > 256 || ny > 256 || nx * ny > 256 || total + static_cast<std::size_t>(nx * ny) > 256) {
                    total = 257;
                    break;
                }
                total += static_cast<std::size_t>(nx * ny);
                canonical.push_back(std::move(tile));
            }
        if (total > 256) continue;
        result.reserve(total);
        for (const auto& tile : canonical) {
            const auto [x0, x1] = repeats(bounds.west, bounds.east, tile.local.west, tile.local.east, world_width);
            const auto [y0, y1] = repeats(bounds.south, bounds.north, tile.local.south, tile.local.north, world_height);
            for (auto x = x0; x <= x1; ++x)
                for (auto y = y0; y <= y1; ++y) {
                    auto copy = tile;
                    copy.local.west += static_cast<double>(x) * world_width;
                    copy.local.east += static_cast<double>(x) * world_width;
                    copy.local.south += static_cast<double>(y) * world_height;
                    copy.local.north += static_cast<double>(y) * world_height;
                    result.push_back(std::move(copy));
                }
        }
        return result;
    }
    return result;
}
}  // namespace kd::view
