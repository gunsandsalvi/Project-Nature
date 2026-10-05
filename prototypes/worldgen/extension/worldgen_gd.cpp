// P7 World generation on the phone (IMPLEMENTATION α0.5a, WLD-11): the generator (../src) as a Godot extension, so
// the prototype app runs the very code the cloud runs, on the phone's own cores. One class, WorldGen, which the app's
// screen makes on its own thread: "New world" timed, the three maps, then settling the first. Pre-production code
// (research 00).
#include <gdextension_interface.h>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include <cstring>
#include <memory>

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
    }

public:
    // "New world" on this many threads (WLD-10): the candidates made and qualified, the seconds each part took, the
    // offered worlds' summaries and every candidate's reasons.
    godot::Dictionary make(int threads) {
        worldgen::Settings settings;
        settings.threads = threads;
        threads_ = threads;
        offer_ = std::make_unique<worldgen::Offer>(worldgen::new_world(settings));
        settled_ = false;
        const worldgen::Offer& o = *offer_;
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
        settled_ = true;
        return worldgen::settle(&offer_->three[0], worldgen::Settings{}.settle_years, &pool);
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

    int map_width() const { return worldgen::Settings{}.full_width / kMapScale; }
    int map_height() const { return worldgen::Settings{}.full_width / 2 / kMapScale; }

    // The offered worlds' checksums, and the first's after settling, in one hash (RES-05).
    godot::String digest() const {
        if (!offer_) {
            return {};
        }
        std::vector<std::uint64_t> sums;
        for (const worldgen::World& w : offer_->three) {
            sums.push_back(w.checksum());
        }
        if (settled_) {
            sums.push_back(offer_->three[0].checksum());
        }
        return {samebits::digest(sums).c_str()};
    }

private:
    std::unique_ptr<worldgen::Offer> offer_;
    int threads_ = 1;
    bool settled_ = false;
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
