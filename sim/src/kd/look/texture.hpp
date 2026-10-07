// A texture's record (A5.4): the catalogue kind `texture`, one entry for each tile of a material and each version of a
// tile, read from the art source's textures/ as the art lane writes them: art/textures/meadow/record.toml is the entry
// art:meadow, art/textures/meadow/middle/v2/record.toml is art:meadow/middle/v2. What it is, its route, its levels with
// their files and digests, where its pictures came from, how it was made, its re-grid's loss, its truth check and your
// approval. Every field affects only the look, so a texture's change changes its source's look digest and nothing else
// (PLT-09). The art lane's checks (tools/art/checks.py) hold each record to the plan's lines in the cloud.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "kd/data/schema.hpp"

namespace kd::look {

/// A texture's level for one band: its file and digest, the level it was made from and how, and the numbers fitted so
/// it matches the first level.
struct TextureLevel {
    std::int64_t level = 0;
    std::string file;
    std::string sha256;
    std::string made_from;
    std::string way;
    std::string regrid_loss;
    std::string calibration;

    template <typename V, typename Self>
    static void visit(V& v, Self& s) {
        using data::Affects;
        v.whole({"level", "which level: 0 is the first band's, each after it half the size", Affects::look}, s.level,
                {0, 12});
        v.text({"file", "its lossless file, by its path from the repository's top", Affects::look}, s.file);
        v.text({"sha256", "its file's SHA-256", Affects::look}, s.sha256);
        v.text({"made_from", "the SHA-256 of the level it was made from, so a change there marks it stale",
                Affects::look, false},
               s.made_from);
        v.text({"way", "how it was made", Affects::look}, s.way);
        v.text({"regrid_loss", "a redrawn level's loss in its re-gridding, such as \"9.7%\"", Affects::look, false},
               s.regrid_loss);
        v.text({"calibration", "the numbers fitted so it matches the first level", Affects::look, false},
               s.calibration);
    }
};

/// Implements PRE-20 and PRE-42, see A5.4: a texture's record.
struct Texture {
    std::string about;
    std::string route;
    std::int64_t tile_texels = 0;
    std::int64_t texels_a_metre = 0;
    std::int64_t first_band = 0;
    std::vector<std::string> sources;
    std::vector<std::string> original_sha256;
    std::vector<std::string> c2pa;
    std::vector<std::string> requests;
    std::string made;
    std::string regrid_loss;
    std::string truth;
    std::string approved;
    std::vector<TextureLevel> levels;

    template <typename V, typename Self>
    static void visit(V& v, Self& s) {
        using data::Affects;
        v.text({"about", "what it is, in a few words", Affects::look}, s.about);
        v.choice({"route", "how it is made: from approved pictures, or by code", Affects::look}, s.route,
                 {"picture", "code"});
        v.whole({"tile_texels", "the tile's width in texture pixels at its first level", Affects::look}, s.tile_texels,
                {1, 4'096});
        v.whole({"texels_a_metre", "texture pixels a metre at its first level: 64 at band 0, half each band after",
                 Affects::look},
                s.texels_a_metre, {1, 64});
        v.whole({"first_band", "the band its first level serves: 0 for a near tile, 2 middle, 4 far", Affects::look},
                s.first_band, {0, 8});
        v.texts({"sources", "each picture it was made from, by its path in art/sources/", Affects::look}, s.sources);
        v.texts({"original_sha256", "each picture's original's SHA-256", Affects::look}, s.original_sha256);
        v.texts({"c2pa", "whether each picture's original carried its C2PA record", Affects::look}, s.c2pa);
        v.texts({"requests", "each picture's request, by its path in art/requests/", Affects::look}, s.requests);
        v.text({"made", "how it was made", Affects::look}, s.made);
        v.text({"regrid_loss", "the first level's loss in its re-gridding, such as \"6.1%\", or why there was none",
                Affects::look},
               s.regrid_loss);
        v.text({"truth", "its truth check: who looked, when, and what they found (A5.6)", Affects::look}, s.truth);
        v.text({"approved", "your words and the date, or \"waiting\"", Affects::look}, s.approved);
        v.records({"band", "its levels, from its first band's, each its own table", Affects::look}, s.levels);
    }
};

}  // namespace kd::look
