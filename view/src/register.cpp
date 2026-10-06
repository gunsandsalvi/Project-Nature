// The extension's entry point (A3.8): Godot calls kindling_init when it loads libkindling, and the classes GDScript
// sees are registered here.
#include <gdextension_interface.h>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include <chrono>

#include "crowd.hpp"
#include "device.hpp"
#include "frames.hpp"
#include "world.hpp"
#include "worlds.hpp"

namespace {

void initialize(godot::ModuleInitializationLevel level) {
    if (level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    godot::ClassDB::register_class<kd::view::KdDevice>();
    godot::ClassDB::register_class<kd::view::KdWorld>();
    godot::ClassDB::register_class<kd::view::KdCrowd>();
    godot::ClassDB::register_class<kd::view::KdWorlds>();
}

void uninitialize(godot::ModuleInitializationLevel /*level*/) {}

// Once a frame, after every node's _process: the frame meter's own clock (A3.9, PLT-04).
void each_frame() {
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    kd::view::frame_meter().frame(std::chrono::duration<double, std::milli>(now).count());
}

}  // namespace

extern "C" GDExtensionBool GDE_EXPORT kindling_init(GDExtensionInterfaceGetProcAddress get_proc_address,
                                                    GDExtensionClassLibraryPtr library,
                                                    GDExtensionInitialization* initialization) {
    godot::GDExtensionBinding::InitObject init(get_proc_address, library, initialization);
    init.register_initializer(initialize);
    init.register_terminator(uninitialize);
    init.register_frame_callback(each_frame);
    init.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);
    return init.init();
}
