// P6 A thousand minds on the phone (IMPLEMENTATION α0.4b, TIM-07, MND-15): the minds (../src) as a Godot extension,
// so the prototype app runs the very code the cloud runs, on the phone's own cores. One class, Minds, which the app's
// screen makes, runs a day at a time on its own thread, and asks for the time each part took. Pre-production code
// (research 00).
#include <gdextension_interface.h>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

#include <chrono>
#include <memory>

#include "hash.hpp"
#include "minds.hpp"

namespace {

class Minds : public godot::RefCounted {
    GDCLASS(Minds, godot::RefCounted)

protected:
    static void _bind_methods() {
        godot::ClassDB::bind_method(godot::D_METHOD("make", "threads"), &Minds::make);
        godot::ClassDB::bind_method(godot::D_METHOD("run_days", "days"), &Minds::run_days);
        godot::ClassDB::bind_method(godot::D_METHOD("stats"), &Minds::stats);
        godot::ClassDB::bind_method(godot::D_METHOD("checksum"), &Minds::checksum);
        godot::ClassDB::bind_method(godot::D_METHOD("explain", "person"), &Minds::explain);
    }

public:
    // A new world of a thousand people, on this many threads: the seconds it took to make.
    double make(int threads) {
        const auto t0 = std::chrono::steady_clock::now();
        minds::Settings settings;
        settings.threads = threads;
        world_ = std::make_unique<minds::World>(settings);
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    }

    // Runs the world on for this many game days: the seconds it took.
    double run_days(int days) {
        if (!world_) {
            return 0.0;
        }
        const auto t0 = std::chrono::steady_clock::now();
        for (int d = 0; d < days; ++d) {
            world_->run_day();
        }
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    }

    // The days run, the people, the decisions taken, the seconds each part of the minds took (summed over threads),
    // and how the trips were found.
    godot::Dictionary stats() const {
        godot::Dictionary out;
        if (!world_) {
            return out;
        }
        const minds::Times& t = world_->times();
        const minds::TripCounts trips = world_->trips();
        out["days"] = world_->day();
        out["people"] = static_cast<std::int64_t>(world_->people().size());
        out["decisions"] = world_->decisions();
        out["cut_short"] = world_->cut_short();
        out["results"] = t.land;
        out["choice"] = t.choice;
        out["paths"] = t.paths;
        out["talk"] = t.talk;
        out["other"] = t.other;
        out["trips_inside"] = trips.inside;
        out["trips_cached"] = trips.cached;
        out["trips_fresh"] = trips.fresh;
        out["trips_none"] = trips.none;
        return out;
    }

    // The whole state's hash, as 16 hexadecimal digits.
    godot::String checksum() const {
        return world_ ? godot::String(samebits::hex(world_->checksum()).c_str()) : godot::String();
    }

    // What someone is doing and why (PRN-13).
    godot::String explain(int person) const {
        if (!world_ || person < 0 || person >= static_cast<int>(world_->people().size())) {
            return {};
        }
        return {world_->explain(person).c_str()};
    }

private:
    std::unique_ptr<minds::World> world_;
};

void initialize(godot::ModuleInitializationLevel level) {
    if (level == godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
        GDREGISTER_CLASS(Minds);
    }
}

void uninitialize(godot::ModuleInitializationLevel /*level*/) {}

}  // namespace

extern "C" {
GDExtensionBool GDE_EXPORT minds_init(GDExtensionInterfaceGetProcAddress get_proc_address,
                                      GDExtensionClassLibraryPtr library, GDExtensionInitialization* initialization) {
    godot::GDExtensionBinding::InitObject init(get_proc_address, library, initialization);
    init.register_initializer(initialize);
    init.register_terminator(uninitialize);
    init.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);
    return init.init();
}
}
