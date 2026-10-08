#include "kd/look/sprite.hpp"

#include <algorithm>
#include <array>
#include <set>
#include <utility>

#include "kd/look/texture.hpp"

namespace kd::look {
void check_sprites(const data::Catalogue& cat, std::vector<data::Problem>& problems) {
    const auto& sprites = cat.kind<SpriteFamily>();
    const auto& textures = cat.kind<Texture>();
    for (std::uint32_t i = 0; i < sprites.size(); ++i) {
        const auto& s = sprites[i];
        const auto refuse = [&](std::string_view field, const std::string& why) {
            problems.push_back(sprites.at(i, field, why));
        };
        for (const auto* text : {&s.about, &s.asset, &s.action, &s.season, &s.part, &s.sheet, &s.approved}) {
            if (text->empty()) {
                refuse("asset", "sprite identity, inspection and approval text must not be empty");
            }
        }
        const std::int64_t expected = s.family == "near" ? 64 : s.family == "middle" ? 16 : s.family == "far" ? 4 : 0;
        if (expected == 0 || s.density != expected) {
            refuse("density", "near, middle and far are independent 64, 16 and 4 pixel families");
        }
        if (s.facings != 1 && s.facings != 4 && s.facings != 8) {
            refuse("facings", "a sprite has one, four or eight authored facings");
        }
        if (s.normal_basis != "world-east-south-up" && s.normal_basis != "world-east-north-up") {
            refuse("normal_basis", "declare the source normals' world east/south/up or east/north/up basis");
        }
        if (s.material_map != "fixture27-v1" && s.material_map != "terrain-v1") {
            refuse("material_map", "unknown categorical material mapping");
        }
        if (s.page_width != s.page_height) {
            refuse("page_height", "the current kdtex format requires square complete pages");
        }
        std::int64_t size = s.page_width;
        std::size_t levels = 1;
        while (size > 1 && size % 2 == 0) {
            size /= 2;
            ++levels;
        }
        if (size != 1) {
            refuse("page_width", "page dimensions must halve exactly to one pixel");
        }
        for (const auto& [field, ref] : std::array<std::pair<const char*, const data::Ref*>, 3>{
                 {{"colour", &s.colour}, {"normal", &s.normal}, {"material", &s.material}}}) {
            if (ref->index >= textures.size()) {
                refuse(field, "channel names no loaded Texture record");
                continue;
            }
            const auto& channel = textures[ref->index];
            if (channel.tile_texels != s.page_width || channel.texels_a_metre != s.density ||
                channel.levels.size() != levels) {
                refuse(field, "channel dimensions, density or complete reduction count differ from the bundle");
            }
            for (std::size_t level = 0; level < channel.levels.size(); ++level) {
                if (channel.levels[level].level != static_cast<std::int64_t>(level)) {
                    refuse(field, "channel reductions must be consecutive from zero");
                }
            }
        }
        std::set<std::pair<std::int64_t, std::int64_t>> cells;
        for (const auto& c : s.cells) {
            if (c.frame >= s.frames || c.facing >= s.facings || !cells.emplace(c.frame, c.facing).second) {
                refuse("cells", "frame and facing cells must be unique and inside their declared counts");
            }
            if (c.x < c.gutter || c.y < c.gutter || c.x + c.width + c.gutter > s.page_width ||
                c.y + c.height + c.gutter > s.page_height || c.trim_x + c.width > c.canvas_width ||
                c.trim_y + c.height > c.canvas_height) {
                refuse("cells", "cell rectangle, gutter and trim must fit their page and original canvas");
            }
        }
        if (static_cast<std::int64_t>(cells.size()) != s.frames * s.facings) {
            refuse("cells", "bundle must cover every declared frame and facing exactly once");
        }
        std::set<std::pair<std::pair<std::int64_t, std::int64_t>, std::string>> attachments;
        for (const auto& a : s.attachments) {
            if (a.name.empty() || !cells.contains({a.frame, a.facing}) ||
                !attachments.emplace(std::pair{a.frame, a.facing}, a.name).second) {
                refuse("attachments", "attachment names must be unique in a declared frame and facing");
            }
        }
        const auto source = sprites.name(i).substr(0, sprites.name(i).find(':'));
        for (std::uint32_t j = 0; j < i; ++j) {
            const auto& other = sprites[j];
            if (source != sprites.name(j).substr(0, sprites.name(j).find(':')) || s.asset != other.asset ||
                s.action != other.action || s.season != other.season || s.family != other.family) {
                continue;
            }
            if (s.part == other.part) {
                refuse("part", "the same asset/action/season/family part is declared twice");
            }
            if (s.frames != other.frames || s.facings != other.facings) {
                refuse("cells", "aligned parts must declare identical frame and facing counts");
            }
            for (const auto& c : s.cells) {
                const auto match = std::find_if(other.cells.begin(), other.cells.end(), [&](const auto& b) {
                    return c.frame == b.frame && c.facing == b.facing;
                });
                if (match != other.cells.end() &&
                    (c.canvas_width != match->canvas_width || c.canvas_height != match->canvas_height ||
                     c.pivot_x_256 != match->pivot_x_256 || c.pivot_y_256 != match->pivot_y_256)) {
                    refuse("cells", "aligned parts must retain their shared untrimmed canvas and pivot");
                }
            }
        }
    }
}
}  // namespace kd::look
