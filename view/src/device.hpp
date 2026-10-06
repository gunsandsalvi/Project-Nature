// The device class (A3.8, A3.9): what GDScript learns about the phone and the simulation's threads on it, for the
// self-check (A2.3) and later the benchmark. It reads and reports; it never decides.
#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_int64_array.hpp>
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
    /// The heat headroom now and forecast 10 s ahead, and the thermal status (Android 11 and later only); the
    /// headroom at which light, moderate and severe throttling begin (light, moderate, severe; Android 15 and
    /// later); and what the phone last pushed to the headroom listener (listener_calls, listener_headroom,
    /// listener_forecast and listener_seconds; Android 16 and later). Implements PLT-04, see A3.9.
    godot::Dictionary thermal() const;
    /// The graphics chip's headroom from 0 to 100, where 0 is no more to give (available, headroom and
    /// min_interval_ms; Android 16 and later). Implements PLT-04, see A3.9.
    godot::Dictionary gpu_headroom() const;
    /// The battery's voltage (voltage_v) and current (current_a) from the kernel, where the system lets the app
    /// read them. Implements PLT-04, see A3.9.
    godot::Dictionary battery_supply() const;
    /// What the graphics driver offers for shading fewer times than once a pixel, asked through a Vulkan instance of
    /// the app's own (A4.7): available, extension, per_draw, per_primitive, from_picture, and the rates, such as
    /// "2x2". Implements VIS-14, see A4.7.
    godot::Dictionary shading_rates() const;
    /// How the storage that holds the app's data is mounted, from /proc/self/mounts.
    godot::String storage() const;
    /// The proof suites' names, in order.
    godot::PackedStringArray proof_suites() const;
    /// One proof suite's digest, run on the given number of simulation threads (A3.4).
    godot::String proof(const godot::String& suite, int threads) const;
    /// The compiler and C++ library the simulation was built with.
    godot::String built_with() const;

    // telemetry, for the benchmark (A3.9, PLT-04)
    /// The clock each core runs at now, in kHz, -1 where the system hides it.
    godot::PackedInt64Array clocks() const;
    /// Each of our own threads, named "kd-...", by its name, with the processor time it has used, in seconds.
    godot::Dictionary thread_times() const;
    /// The memory the app holds now and the most it has held, in megabytes: resident_mb and peak_mb.
    godot::Dictionary memory() const;
    /// A section of the phone's System Tracing, begun and ended on the same thread; nothing off Android.
    void trace_begin(const godot::String& name) const;
    void trace_end() const;
    /// The frames by our own measure (A3.9), counted afresh from now, for a frame period and the screen's refresh.
    void frames_reset(double period_ms, double refresh_hz) const;
    /// The frames since: frames, on_time, stalls, late and slowest_ms.
    godot::Dictionary frames() const;

    // the benchmark (A18.1), as the simulation lists it, and its code
    /// Each scenario, in order: name, about, ground, speed, camera, pinned, saves, seconds, mark, call_at, call_camp,
    /// and its pass lines on_time (thousandths), slowest and open (milliseconds), 0 where it has none.
    godot::Array bench_scenarios() const;
    /// The code for these measures, by the code's field names (Implements PLT-04).
    godot::String bench_code(const godot::Dictionary& values) const;
    /// A code read back: values, and why it could not be read, empty when it could.
    godot::Dictionary bench_read(const godot::String& code) const;

protected:
    static void _bind_methods();
};

}  // namespace kd::view
