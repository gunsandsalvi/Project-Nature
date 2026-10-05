// P7 World generation on the phone (IMPLEMENTATION α0.5a, WLD-11): the generator (../src) as a Godot extension, so
// the prototype app runs the very code the cloud runs, on the phone's own cores. One class, WorldGen, which the app's
// screen makes on its own thread: "New world" timed, the three maps, then settling the first. Pre-production code
// (research 00).
#include <gdextension_interface.h>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include <algorithm>
#include <cstring>
#include <map>
#include <memory>

#include "area.hpp"
#include "hash.hpp"
#include "world.hpp"

namespace {

constexpr int kMapScale = 4;  // a map pixel for 4 × 4 cells: 512 × 256

class WorldGen : public godot::RefCounted {
    GDCLASS(WorldGen, godot::RefCounted)

protected:
    static void _bind_methods() {
        godot::ClassDB::bind_method(godot::D_METHOD("make", "threads"), &WorldGen::make);
        godot::ClassDB::bind_method(godot::D_METHOD("settle"), &WorldGen::settle);
        godot::ClassDB::bind_method(godot::D_METHOD("map", "world"), &WorldGen::map);
        godot::ClassDB::bind_method(godot::D_METHOD("map_width"), &WorldGen::map_width);
        godot::ClassDB::bind_method(godot::D_METHOD("map_height"), &WorldGen::map_height);
        godot::ClassDB::bind_method(godot::D_METHOD("digest"), &WorldGen::digest);
        godot::ClassDB::bind_method(godot::D_METHOD("map_at", "world", "scale"), &WorldGen::map_at);
        godot::ClassDB::bind_method(godot::D_METHOD("heights", "world"), &WorldGen::heights);
        godot::ClassDB::bind_method(godot::D_METHOD("ground", "world", "east", "north", "n", "spacing"),
                                    &WorldGen::ground);
        godot::ClassDB::bind_method(godot::D_METHOD("start", "world"), &WorldGen::start);
        godot::ClassDB::bind_method(godot::D_METHOD("ground_mesh", "world", "east", "north", "n", "spacing"),
                                    &WorldGen::ground_mesh);
        godot::ClassDB::bind_method(godot::D_METHOD("trees", "world", "east", "north", "side", "spacing"),
                                    &WorldGen::trees);
        godot::ClassDB::bind_method(godot::D_METHOD("rivers", "world", "least_km2"), &WorldGen::rivers);
        godot::ClassDB::bind_method(godot::D_METHOD("coasts", "world"), &WorldGen::coasts);
        godot::ClassDB::bind_method(godot::D_METHOD("world_size"), &WorldGen::world_size);
        godot::ClassDB::bind_method(godot::D_METHOD("world_cells"), &WorldGen::world_cells);
        godot::ClassDB::bind_method(godot::D_METHOD("tilt", "world"), &WorldGen::tilt);
        godot::ClassDB::bind_method(godot::D_METHOD("prepare_ground", "world"), &WorldGen::prepare_ground);
        godot::ClassDB::bind_method(godot::D_METHOD("chunk", "world", "east", "north", "n", "spacing"),
                                    &WorldGen::chunk);
        godot::ClassDB::bind_method(godot::D_METHOD("climate_texture", "world"), &WorldGen::climate_texture);
        godot::ClassDB::bind_method(godot::D_METHOD("cover_texture", "world"), &WorldGen::cover_texture);
        godot::ClassDB::bind_method(godot::D_METHOD("water_texture", "world"), &WorldGen::water_texture);
        godot::ClassDB::bind_method(godot::D_METHOD("cloud_noise", "size"), &WorldGen::cloud_noise);
    }

public:
    // "New world" on this many threads (WLD-10): the candidates made and qualified, the seconds each part took, the
    // offered worlds' summaries and every candidate's reasons.
    godot::Dictionary make(int threads) {
        worldgen::Settings settings;
        settings.threads = threads;
        threads_ = threads;
        offer_ = std::make_unique<worldgen::Offer>(worldgen::new_world(settings));
        const worldgen::Offer& o = *offer_;
        sums_.clear();
        for (const worldgen::World& w : o.three) {
            sums_.push_back(w.checksum());
        }
        godot::Dictionary out;
        out["made"] = o.made;
        out["qualified"] = o.qualified;
        out["best"] = settings.best;
        out["candidates_seconds"] = o.candidates_seconds;
        out["best_seconds"] = o.best_seconds;
        out["plates"] = o.times.plates;
        out["erosion"] = o.times.erosion;
        out["climate"] = o.times.climate;
        out["life"] = o.times.life;
        out["score"] = o.times.score;
        godot::PackedStringArray summaries;
        for (const worldgen::World& w : o.three) {
            summaries.push_back(w.summary.c_str());
        }
        out["summaries"] = summaries;
        godot::PackedStringArray log;
        for (const std::string& line : o.log) {
            log.push_back(line.c_str());
        }
        out["log"] = log;
        return out;
    }

    // Settles the first offered world for 10 years (WLD-08): the seconds it took.
    double settle() {
        if (!offer_ || offer_->three.empty()) {
            return 0.0;
        }
        minds::Pool pool(threads_);
        const double seconds = worldgen::settle(&offer_->three[0], worldgen::Settings{}.settle_years, &pool);
        sums_.push_back(offer_->three[0].checksum());
        return seconds;
    }

    // An offered world's map as RGB bytes, north at the top.
    godot::PackedByteArray map(int world) const {
        godot::PackedByteArray out;
        if (!offer_ || world < 0 || world >= static_cast<int>(offer_->three.size())) {
            return out;
        }
        const std::vector<std::uint8_t> rgb =
            worldgen::map_rgb(offer_->three[static_cast<std::size_t>(world)], kMapScale);
        out.resize(static_cast<std::int64_t>(rgb.size()));
        std::memcpy(out.ptrw(), rgb.data(), rgb.size());
        return out;
    }

    // P8 The zoom (IMPLEMENTATION α0.5b) reads a world through these: its map a pixel a cell or coarser, without the
    // rivers it draws as lines, its cells' heights, the ground anywhere at any spacing (A7.5), and where its start
    // region lies.
    godot::PackedByteArray map_at(int world, int scale) const {
        godot::PackedByteArray out;
        const worldgen::World* w = offered(world);
        if (w == nullptr || scale < 1) {
            return out;
        }
        const std::vector<std::uint8_t> rgb = worldgen::map_rgb(*w, scale, false);
        out.resize(static_cast<std::int64_t>(rgb.size()));
        std::memcpy(out.ptrw(), rgb.data(), rgb.size());
        return out;
    }

    godot::PackedFloat32Array heights(int world) const {
        godot::PackedFloat32Array out;
        const worldgen::World* w = offered(world);
        if (w == nullptr) {
            return out;
        }
        out.resize(static_cast<std::int64_t>(w->height.size()));
        std::memcpy(out.ptrw(), w->height.data(), w->height.size() * sizeof(float));
        return out;
    }

    godot::PackedFloat32Array ground(int world, double east, double north, int n, double spacing) const {
        godot::PackedFloat32Array out;
        const worldgen::World* w = offered(world);
        if (w == nullptr || n < 1) {
            return out;
        }
        const std::vector<float> h = worldgen::ground_heights(*w, east, north, n, spacing);
        out.resize(static_cast<std::int64_t>(h.size()));
        std::memcpy(out.ptrw(), h.data(), h.size() * sizeof(float));
        return out;
    }

    // A square of ground as a mesh's arrays, for P8's chunks: n × n vertices `spacing` metres apart from (east,
    // north), in metres from that corner, x east, y up and z south, the sea flat at its level; their normals; and
    // their colours.
    godot::Array ground_mesh(int world, double east, double north, int n, double spacing) const {
        godot::Array out;
        const worldgen::World* w = offered(world);
        if (w == nullptr || n < 2) {
            return out;
        }
        const std::vector<float> h = worldgen::ground_heights(*w, east, north, n, spacing);
        const std::vector<std::uint8_t> rgb = worldgen::ground_colours(*w, east, north, n, spacing, h);
        godot::PackedVector3Array vertices;
        godot::PackedVector3Array normals;
        godot::PackedColorArray colours;
        const auto count = static_cast<std::int64_t>(n) * n;
        vertices.resize(count);
        normals.resize(count);
        colours.resize(count);
        const auto at = [n, &h](int i, int j) {
            i = std::clamp(i, 0, n - 1);
            j = std::clamp(j, 0, n - 1);
            return std::max(
                0.0F, h[(static_cast<std::size_t>(j) * static_cast<std::size_t>(n)) + static_cast<std::size_t>(i)]);
        };
        for (int j = 0; j < n; ++j) {
            for (int i = 0; i < n; ++i) {
                const std::int64_t k = (static_cast<std::int64_t>(j) * n) + i;
                vertices.set(
                    k, godot::Vector3(static_cast<float>(i * spacing), at(i, j), static_cast<float>(-j * spacing)));
                const float dx = (at(i + 1, j) - at(i - 1, j)) / static_cast<float>(2.0 * spacing);
                const float dz = (at(i, j + 1) - at(i, j - 1)) / static_cast<float>(2.0 * spacing);
                normals.set(k, godot::Vector3(-dx, 1.0F, dz).normalized());
                const std::size_t c = static_cast<std::size_t>(k) * 3;
                colours.set(k,
                            godot::Color(static_cast<float>(rgb[c]) / 255.0F, static_cast<float>(rgb[c + 1]) / 255.0F,
                                         static_cast<float>(rgb[c + 2]) / 255.0F));
            }
        }
        out.push_back(vertices);
        out.push_back(normals);
        out.push_back(colours);
        return out;
    }

    // The trees in a square, each as metres east of its corner, the ground's height, and metres north (A9).
    godot::PackedVector3Array trees(int world, double east, double north, double side, double spacing) const {
        godot::PackedVector3Array out;
        const worldgen::World* w = offered(world);
        if (w == nullptr) {
            return out;
        }
        const std::vector<float> xyh = worldgen::ground_trees(*w, east, north, side, spacing);
        out.resize(static_cast<std::int64_t>(xyh.size() / 3));
        for (std::size_t i = 0; i < xyh.size() / 3; ++i) {
            out.set(static_cast<std::int64_t>(i), godot::Vector3(xyh[i * 3], xyh[(i * 3) + 2], xyh[(i * 3) + 1]));
        }
        return out;
    }

    // The rivers draining at least `least_km2`, as line segments between cell middles in metres, each pair a segment
    // from a cell to where its water goes (A8.5).
    godot::PackedVector2Array rivers(int world, double least_km2) const {
        godot::PackedVector2Array out;
        const worldgen::World* w = offered(world);
        if (w == nullptr) {
            return out;
        }
        const worldgen::Grid& g = w->grid;
        for (const std::int32_t c : w->order) {
            if (w->area[static_cast<std::size_t>(c)] < least_km2) {
                continue;
            }
            const std::int32_t r = w->receiver[static_cast<std::size_t>(c)];
            out.push_back(godot::Vector2(static_cast<float>((g.x_of(c) + 0.5) * g.metres),
                                         static_cast<float>((g.y_of(c) + 0.5) * g.metres)));
            // the receiver beside it, on this side of the torus's edges
            int dx = g.x_of(r) - g.x_of(c);
            int dy = g.y_of(r) - g.y_of(c);
            dx = dx > 1 ? dx - g.width : (dx < -1 ? dx + g.width : dx);
            dy = dy > 1 ? dy - g.height : (dy < -1 ? dy + g.height : dy);
            out.push_back(godot::Vector2(static_cast<float>((g.x_of(c) + dx + 0.5) * g.metres),
                                         static_cast<float>((g.y_of(c) + dy + 0.5) * g.metres)));
        }
        return out;
    }

    // The shore as line segments along the cell edges between land and sea, in metres (PRE-29, A8.5).
    godot::PackedVector2Array coasts(int world) const {
        godot::PackedVector2Array out;
        const worldgen::World* w = offered(world);
        if (w == nullptr) {
            return out;
        }
        const worldgen::Grid& g = w->grid;
        const auto m = static_cast<float>(g.metres);
        for (int c = 0; c < g.cells(); ++c) {
            if (w->sea(c)) {
                continue;
            }
            const auto x = static_cast<float>(g.x_of(c));
            const auto y = static_cast<float>(g.y_of(c));
            if (w->sea(g.at(g.x_of(c) + 1, g.y_of(c)))) {
                out.push_back(godot::Vector2((x + 1) * m, y * m));
                out.push_back(godot::Vector2((x + 1) * m, (y + 1) * m));
            }
            if (w->sea(g.at(g.x_of(c) - 1, g.y_of(c)))) {
                out.push_back(godot::Vector2(x * m, y * m));
                out.push_back(godot::Vector2(x * m, (y + 1) * m));
            }
            if (w->sea(g.at(g.x_of(c), g.y_of(c) + 1))) {
                out.push_back(godot::Vector2(x * m, (y + 1) * m));
                out.push_back(godot::Vector2((x + 1) * m, (y + 1) * m));
            }
            if (w->sea(g.at(g.x_of(c), g.y_of(c) - 1))) {
                out.push_back(godot::Vector2(x * m, y * m));
                out.push_back(godot::Vector2((x + 1) * m, y * m));
            }
        }
        return out;
    }

    // The middle of the start region, metres east and north of the world's corner.
    godot::Vector2 start(int world) const {
        const worldgen::World* w = offered(world);
        if (w == nullptr || w->start.cell < 0) {
            return {};
        }
        const worldgen::Grid& g = w->grid;
        return {static_cast<float>((g.x_of(w->start.cell) + 0.5) * g.metres),
                static_cast<float>((g.y_of(w->start.cell) + 0.5) * g.metres)};
    }

    // The world's metres around and from pole to pole (WLD-03), and its cells at full size.
    godot::Vector2 world_size() const {
        return {static_cast<float>(worldgen::kAroundMetres), static_cast<float>(worldgen::kAroundMetres / 2.0)};
    }

    godot::Vector2i world_cells() const {
        const worldgen::Settings s;
        return {s.full_width, s.full_width / 2};
    }

    // The world's tilt, degrees, which sets its seasons (WLD-06).
    double tilt(int world) const {
        const worldgen::World* w = offered(world);
        return w == nullptr ? 0.0 : w->tilt;
    }

    // P8's descent (IMPLEMENTATION α0.5b, A8.1): the world's heights averaged for ground seen from far off, made once
    // before any chunk is asked for, so chunks made on worker threads at once only read them.
    void prepare_ground(int world) {
        const worldgen::World* w = offered(world);
        if (w != nullptr) {
            mips_[world] = worldgen::height_mips(*w);
        }
    }

    // A chunk of ground for P8's descent as a mesh's arrays: n × n points `spacing` metres apart from (east, north), in
    // metres from that corner (x east, y up, z south), then a skirt hanging below each edge so no gap shows where
    // chunks of two levels meet; their normals; and, four numbers a point each, what it morphs into: the height of the
    // grid half as fine where it slides to, its true height and that grid's, and that grid's normal (A8.1, CDLOD);
    // last, the lowest and highest the chunk is drawn, morphing or not, for the camera to leave out what it cannot see.
    godot::Array chunk(int world, double east, double north, int n, double spacing) const {
        godot::Array out;
        const worldgen::World* w = offered(world);
        const auto found = mips_.find(world);
        if (w == nullptr || found == mips_.end() || n < 3 || n % 2 == 0) {
            return out;
        }
        const worldgen::GroundChunk g = worldgen::ground_chunk(*w, found->second, east, north, n, spacing);
        const auto points = static_cast<std::int64_t>(n) * n;
        const std::int64_t count = points + (4LL * n);
        godot::PackedVector3Array vertices;
        godot::PackedVector3Array normals;
        godot::PackedFloat32Array morph;
        godot::PackedFloat32Array morph_normal;
        vertices.resize(count);
        normals.resize(count);
        morph.resize(count * 4);
        morph_normal.resize(count * 4);
        const auto skirt = static_cast<float>((2.0 * spacing) + 1.0);
        const auto put = [&](std::int64_t at, int i, int j, float drop) {
            const auto k = static_cast<std::size_t>((static_cast<std::int64_t>(j) * n) + i);
            vertices.set(at, godot::Vector3(static_cast<float>(i * spacing), g.surface[k] - drop,
                                            static_cast<float>(-j * spacing)));
            normals.set(at, godot::Vector3(g.normal[k * 3], g.normal[(k * 3) + 1], g.normal[(k * 3) + 2]));
            morph.set(at * 4, g.to_surface[k] - drop);
            morph.set((at * 4) + 1, g.truth[k]);
            morph.set((at * 4) + 2, g.to_truth[k]);
            morph.set((at * 4) + 3, drop);
            morph_normal.set(at * 4, g.to_normal[k * 3]);
            morph_normal.set((at * 4) + 1, g.to_normal[(k * 3) + 1]);
            morph_normal.set((at * 4) + 2, g.to_normal[(k * 3) + 2]);
            morph_normal.set((at * 4) + 3, 0.0F);
        };
        float low = g.surface[0];
        float high = g.surface[0];
        for (int j = 0; j < n; ++j) {
            for (int i = 0; i < n; ++i) {
                const auto k = static_cast<std::size_t>((static_cast<std::int64_t>(j) * n) + i);
                low = std::min({low, g.surface[k], g.to_surface[k]});
                high = std::max({high, g.surface[k], g.to_surface[k]});
                put((static_cast<std::int64_t>(j) * n) + i, i, j, 0.0F);
            }
        }
        // the skirts: the south edge, the north, the west and the east, each from its first point
        for (int k = 0; k < n; ++k) {
            put(points + k, k, 0, skirt);
            put(points + n + k, k, n - 1, skirt);
            put(points + (2LL * n) + k, 0, k, skirt);
            put(points + (3LL * n) + k, n - 1, k, skirt);
        }
        out.push_back(vertices);
        out.push_back(normals);
        out.push_back(morph);
        out.push_back(morph_normal);
        out.push_back(godot::Vector2(low, high));
        return out;
    }

    // The world's cells as textures for P8's picture (A8.5): see worldgen::climate_texture and the rest.
    godot::PackedFloat32Array climate_texture(int world) const {
        godot::PackedFloat32Array out;
        const worldgen::World* w = offered(world);
        if (w != nullptr) {
            const std::vector<float> v = worldgen::climate_texture(*w);
            out.resize(static_cast<std::int64_t>(v.size()));
            std::memcpy(out.ptrw(), v.data(), v.size() * sizeof(float));
        }
        return out;
    }

    godot::PackedByteArray cover_texture(int world) const {
        const worldgen::World* w = offered(world);
        return w == nullptr ? godot::PackedByteArray() : bytes(worldgen::cover_texture(*w));
    }

    godot::PackedByteArray water_texture(int world) const {
        const worldgen::World* w = offered(world);
        return w == nullptr ? godot::PackedByteArray() : bytes(worldgen::water_texture(*w));
    }

    // Noise for P8's clouds (A8.6), the same for every world.
    godot::PackedByteArray cloud_noise(int size) const { return bytes(worldgen::cloud_noise(1, size)); }

    int map_width() const { return worldgen::Settings{}.full_width / kMapScale; }
    int map_height() const { return worldgen::Settings{}.full_width / 2 / kMapScale; }

    // The offered worlds' checksums as made, and the first's after settling, in one hash, as the command line
    // prints it (RES-05).
    godot::String digest() const { return offer_ ? godot::String(samebits::digest(sums_).c_str()) : godot::String(); }

private:
    static godot::PackedByteArray bytes(const std::vector<std::uint8_t>& v) {
        godot::PackedByteArray out;
        out.resize(static_cast<std::int64_t>(v.size()));
        std::memcpy(out.ptrw(), v.data(), v.size());
        return out;
    }

    const worldgen::World* offered(int world) const {
        if (!offer_ || world < 0 || world >= static_cast<int>(offer_->three.size())) {
            return nullptr;
        }
        return &offer_->three[static_cast<std::size_t>(world)];
    }

    std::unique_ptr<worldgen::Offer> offer_;
    std::vector<std::uint64_t> sums_;
    std::map<int, worldgen::HeightMips> mips_;
    int threads_ = 1;
};

void initialize(godot::ModuleInitializationLevel level) {
    if (level == godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
        GDREGISTER_CLASS(WorldGen);
    }
}

void uninitialize(godot::ModuleInitializationLevel /*level*/) {}

}  // namespace

extern "C" {
GDExtensionBool GDE_EXPORT worldgen_init(GDExtensionInterfaceGetProcAddress get_proc_address,
                                         GDExtensionClassLibraryPtr library,
                                         GDExtensionInitialization* initialization) {
    godot::GDExtensionBinding::InitObject init(get_proc_address, library, initialization);
    init.register_initializer(initialize);
    init.register_terminator(uninitialize);
    init.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);
    return init.init();
}
}
