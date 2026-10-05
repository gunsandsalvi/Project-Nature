// P5 The same bits on the phone (IMPLEMENTATION α0.4a, RES-05): the toy world (../src) as a Godot extension, so the
// prototype app runs the very code the cloud runs, on the phone's own chip. One class, SameBits, with static methods
// for the app's screen. Pre-production code (research 00).
#include <gdextension_interface.h>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include "expected.hpp"
#include "world.hpp"

namespace {

class SameBits : public godot::RefCounted {
    GDCLASS(SameBits, godot::RefCounted)

protected:
    static void _bind_methods() {
        godot::ClassDB::bind_static_method("SameBits", godot::D_METHOD("run", "threads", "days"), &SameBits::run);
        godot::ClassDB::bind_static_method("SameBits", godot::D_METHOD("cloud_digest"), &SameBits::cloud_digest);
        godot::ClassDB::bind_static_method("SameBits", godot::D_METHOD("days"), &SameBits::days);
    }

public:
    // The toy world's default run on this many threads for this many days: its digest, then each day's checksum.
    static godot::PackedStringArray run(int threads, int days) {
        samebits::Settings settings;
        settings.threads = threads;
        const auto sums = samebits::run(settings, days);
        godot::PackedStringArray out;
        out.push_back(godot::String(samebits::digest(sums).c_str()));
        for (const std::uint64_t s : sums) {
            out.push_back(godot::String(samebits::hex(s).c_str()));
        }
        return out;
    }

    // The digest the cloud recorded for the default run of days() days, on x86-64.
    static godot::String cloud_digest() { return {samebits::kCloudDigest}; }

    static int days() { return samebits::kDays; }
};

void initialize(godot::ModuleInitializationLevel level) {
    if (level == godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
        GDREGISTER_CLASS(SameBits);
    }
}

void uninitialize(godot::ModuleInitializationLevel /*level*/) {}

}  // namespace

extern "C" {
GDExtensionBool GDE_EXPORT samebits_init(GDExtensionInterfaceGetProcAddress get_proc_address,
                                         GDExtensionClassLibraryPtr library,
                                         GDExtensionInitialization* initialization) {
    godot::GDExtensionBinding::InitObject init(get_proc_address, library, initialization);
    init.register_initializer(initialize);
    init.register_terminator(uninitialize);
    init.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);
    return init.init();
}
}
