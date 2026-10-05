// The extension's entry point (A3.8): Godot calls kindling_init when it loads libkindling, and the classes GDScript
// sees are registered here.
#include <gdextension_interface.h>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "device.hpp"

namespace {

void initialize(godot::ModuleInitializationLevel level) {
    if (level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    godot::ClassDB::register_class<kd::view::KdDevice>();
}

void uninitialize(godot::ModuleInitializationLevel /*level*/) {}

}  // namespace

extern "C" GDExtensionBool GDE_EXPORT kindling_init(GDExtensionInterfaceGetProcAddress get_proc_address,
                                                    GDExtensionClassLibraryPtr library,
                                                    GDExtensionInitialization* initialization) {
    godot::GDExtensionBinding::InitObject init(get_proc_address, library, initialization);
    init.register_initializer(initialize);
    init.register_terminator(uninitialize);
    init.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);
    return init.init();
}
