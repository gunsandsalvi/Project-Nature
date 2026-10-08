// T2.9a.1: one authored family and its aligned channels in the shared look catalogue (A3.6, A5.3).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "kd/data/catalogue.hpp"
#include "kd/data/schema.hpp"

namespace kd::look {

/// A frame rectangle and exact shared pivot, before reduction or optional packing.
struct SpriteCell {
    std::int64_t frame = 0;
    std::int64_t facing = 0;
    std::int64_t x = 0;
    std::int64_t y = 0;
    std::int64_t width = 0;
    std::int64_t height = 0;
    std::int64_t pivot_x_256 = 0;
    std::int64_t pivot_y_256 = 0;
    std::int64_t canvas_width = 0;
    std::int64_t canvas_height = 0;
    std::int64_t trim_x = 0;
    std::int64_t trim_y = 0;
    std::int64_t gutter = 0;

    template <typename V, typename Self>
    static void visit(V& v, Self& s) {
        using data::Affects;
        v.whole({"frame", "zero-based action frame", Affects::look}, s.frame, {0, 255});
        v.whole({"facing", "zero-based facing", Affects::look}, s.facing, {0, 7});
        v.whole({"x", "rectangle left in page pixels", Affects::look}, s.x, {0, 4096});
        v.whole({"y", "rectangle top in page pixels", Affects::look}, s.y, {0, 4096});
        v.whole({"width", "rectangle width in page pixels", Affects::look}, s.width, {1, 4096});
        v.whole({"height", "rectangle height in page pixels", Affects::look}, s.height, {1, 4096});
        v.whole({"pivot_x_256", "untrimmed horizontal pivot in 1/256 source pixel", Affects::look}, s.pivot_x_256,
                {-1048576, 1048576});
        v.whole({"pivot_y_256", "untrimmed vertical pivot in 1/256 source pixel", Affects::look}, s.pivot_y_256,
                {-1048576, 1048576});
        v.whole({"canvas_width", "untrimmed frame width", Affects::look}, s.canvas_width, {1, 4096});
        v.whole({"canvas_height", "untrimmed frame height", Affects::look}, s.canvas_height, {1, 4096});
        v.whole({"trim_x", "crop left in untrimmed frame", Affects::look}, s.trim_x, {0, 4096});
        v.whole({"trim_y", "crop top in untrimmed frame", Affects::look}, s.trim_y, {0, 4096});
        v.whole({"gutter", "extruded allocation outside the rectangle", Affects::look}, s.gutter, {0, 64});
    }
};

/// A named attachment position; it does not approve any attached artwork.
struct SpriteAttachment {
    std::string name;
    std::int64_t frame = 0;
    std::int64_t facing = 0;
    std::int64_t x_256 = 0;
    std::int64_t y_256 = 0;
    std::int64_t order = 0;

    template <typename V, typename Self>
    static void visit(V& v, Self& s) {
        using data::Affects;
        v.text({"name", "attachment point name", Affects::look}, s.name);
        v.whole({"frame", "action frame", Affects::look}, s.frame, {0, 255});
        v.whole({"facing", "facing", Affects::look}, s.facing, {0, 7});
        v.whole({"x_256", "horizontal attachment in 1/256 source pixel", Affects::look}, s.x_256, {-1048576, 1048576});
        v.whole({"y_256", "vertical attachment in 1/256 source pixel", Affects::look}, s.y_256, {-1048576, 1048576});
        v.whole({"order", "drawing order", Affects::look}, s.order, {-4096, 4096});
    }
};

/// Implements PRE-22, PRE-42, PRE-46 and PLT-09: one immutable aligned authored bundle.
struct SpriteFamily {
    std::string about;
    std::string asset;
    std::string action;
    std::string season;
    std::string part;
    std::string family;
    std::int64_t density = 0;
    data::Ref colour;
    data::Ref normal;
    data::Ref material;
    std::int64_t page_width = 0;
    std::int64_t page_height = 0;
    std::int64_t frames = 0;
    std::int64_t facings = 0;
    std::string normal_basis;
    std::string material_map;
    std::string sheet;
    std::string approved;
    std::vector<SpriteCell> cells;
    std::vector<SpriteAttachment> attachments;

    template <typename V, typename Self>
    static void visit(V& v, Self& s) {
        using data::Affects;
        v.text({"about", "what the bundle depicts", Affects::look}, s.about);
        v.text({"asset", "stable asset name", Affects::look}, s.asset);
        v.text({"action", "action name, static for a still", Affects::look}, s.action);
        v.text({"season", "season name, all for season independent", Affects::look}, s.season);
        v.text({"part", "named part, whole for an unsplit asset", Affects::look}, s.part);
        v.text({"family", "independent authored near, middle or far family", Affects::look}, s.family);
        v.whole({"density", "first level source pixels per metre", Affects::look}, s.density, {4, 64});
        v.link({"colour", "aligned colour Texture record", Affects::look}, s.colour, "textures");
        v.link({"normal", "aligned normal Texture record", Affects::look}, s.normal, "textures");
        v.link({"material", "aligned material Texture record", Affects::look}, s.material, "textures");
        v.whole({"page_width", "first-level page width", Affects::look}, s.page_width, {1, 4096});
        v.whole({"page_height", "first-level page height", Affects::look}, s.page_height, {1, 4096});
        v.whole({"frames", "action frame count", Affects::look}, s.frames, {1, 256});
        v.whole({"facings", "supported facing count: one, four or eight", Affects::look}, s.facings, {1, 8});
        v.text({"normal_basis", "truthful source normal basis", Affects::look}, s.normal_basis);
        v.text({"material_map", "named exact categorical material mapping", Affects::look}, s.material_map);
        v.text({"sheet", "source sheet path for inspection", Affects::look}, s.sheet);
        v.text({"approved", "source approval words or explicit pending status", Affects::look}, s.approved);
        v.records({"cells", "complete frame and facing rectangles", Affects::look}, s.cells);
        v.records({"attachments", "named attachment points", Affects::look, false}, s.attachments);
    }
};

void check_sprites(const data::Catalogue& cat, std::vector<data::Problem>& problems);

}  // namespace kd::look
