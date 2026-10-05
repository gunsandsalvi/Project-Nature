// The device class (A3.8, A3.9): what GDScript learns about the phone and the simulation's threads on it, for the
// self-check (A2.3) and later the benchmark. It reads and reports; it never decides.
#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace kd::view {

/// Implements PLT-01, see A3.9: the phone's cores, its heat and the simulation threads' state, as GDScript sees them.
class KdDevice : public godot::RefCounted {
    GDCLASS(KdDevice, godot::RefCounted)

public:
    /// Every core with its top clock in kHz, as [{cpu, max_khz}], or empty where the system hides them.
    godot::Array cores() const;
    /// The cores whose top clock is neither the highest nor the lowest: the middle cores (PLT-01).
    godot::PackedInt32Array middle_cores() const;
    /// What a new simulation thread finds and sets: the inherited control register, whether the default holds
    /// once set, and its stack size.
    godot::Dictionary thread_check() const;
    /// The heat headroom now and forecast 10 s ahead, and the thermal status (Android 11 and later only).
    godot::Dictionary thermal() const;
    /// How the storage that holds the app's data is mounted, from /proc/self/mounts.
    godot::String storage() const;
    /// The proof suites' names, in order.
    godot::PackedStringArray proof_suites() const;
    /// One proof suite's digest, run on the given number of simulation threads (A3.4).
    godot::String proof(const godot::String& suite, int threads) const;
    /// The compiler and C++ library the simulation was built with.
    godot::String built_with() const;

protected:
    static void _bind_methods();
};

}  // namespace kd::view
