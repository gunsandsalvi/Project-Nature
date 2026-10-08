// The world class (A3.8): the world as GDScript sees it. For now, until the world itself exists, its catalogue, read
// from the build's copy of data/ (A3.6), the clockwork stand-in or the demonstration's crowd on its runner, and the
// speed loop between it and the screen (A3.9); later make, open, save and commands join it here.
#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_float64_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_int64_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include "crowd_core.hpp"
#include "heat.hpp"
#include "kd/data/catalogue.hpp"
#include "kd/demo/clockwork.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/run/marked.hpp"
#include "kd/run/runner.hpp"
#include "kd/save/files.hpp"
#include "kd/save/keeper.hpp"
#include "pace.hpp"
#include "projection.hpp"
#include "time_requests.hpp"

namespace kd::view {

/// Implements TIM-01, TIM-10 and PLT-01, see A3.8 and A3.9: the world on its own thread, and the screen's time
/// following it at the speed asked.
class KdWorld : public godot::RefCounted {
    GDCLASS(KdWorld, godot::RefCounted)

public:
    /// The share of what the phone can do now that the screen asks of the crowd at most, so the world stays ahead
    /// and the screen glides instead of catching it (A3.9).
    static constexpr double kUse = 0.9;

    KdWorld();
    ~KdWorld() override;

    /// Loads the world's catalogue from these files under res://data/, each read through Godot's FileAccess and
    /// handed to the simulation as bytes (A3.6): its problems, sources with their digests, and kinds with their
    /// entries, and how many files and bytes it read in how many microseconds. Implements MAT-13.
    godot::Dictionary load_catalogue(const godot::PackedStringArray& paths);
    /// An entry's values by its kind's folder and its name, following renames: whole numbers in base units, chances
    /// in parts per million, texts, truth values and lists of names; empty when there is no such entry.
    godot::Dictionary entry(const godot::String& folder, const godot::String& name) const;
    /// Implements PRE-22/PRE-46: owned validated family metadata with nested cells and attachment records.
    godot::Array sprite_families() const;
    /// Implements PLT-04: validated catalogue caps, with the named resident partitions.
    godot::Dictionary stream_limits() const;
    godot::Dictionary navigation_tuning() const;
    void enable_time_requests(bool enabled);
    void set_zoom_density(double density);
    void set_manual_rate(double rate);
    void clear_manual_rate();
    void set_speed_lock(bool locked);
    godot::Dictionary time_requests() const;

    /// Starts the calendar's stand-in world at Year 1, spring, day 1, doing its work each game hour (MAT-16).
    void start_clockwork();
    /// Starts the demonstration's crowd from the loaded catalogue (MAT-16): from a seed, with camps camps, or the
    /// tuning's number when 0; kept nowhere, for the tests.
    void start_crowd(int64_t seed, int64_t camps);
    /// Opens the crowd's world kept in a folder, an absolute path, or makes it there from a seed and camps (A3.7), as
    /// this version of the app, such as "α1.4b": whether it was made, the snapshot it opened, the commands acted again,
    /// the damaged files set aside, the game second it had got to and catches up to, whether another version saved it
    /// and how big an update this one is for it ("none", "small" or "big"), the migrations made to it, and a problem
    /// if it could not open. Implements TIM-05, PLT-07 and PLT-09.
    godot::Dictionary open_crowd(const godot::String& folder, int64_t seed, int64_t camps, const godot::String& build);
    /// Saves the world as it runs, between two batches, its snapshot written on another thread, and checks the free
    /// space where it is kept (A3.7, PLT-10).
    godot::Dictionary open_camp(const godot::String& folder, int64_t seed, const godot::String& build);
    godot::Array people() const;
    godot::Dictionary camp_alpha() const;
    void save();
    /// Saves the world at once, as the app leaves the screen: it stops after the batch it is in, a pause mark is
    /// synced to the journal, and the snapshot is written before this returns (TIM-05).
    void save_now();
    /// Your command to call one of the camps home, by its number in id order, acting at the world's frontier and
    /// written to the journal before it acts (A3.8).
    void call_home(int64_t camp);
    /// The number of the camp nearest a place in world centimetres, within a distance, or -1.
    int64_t nearest_camp(int64_t east, int64_t north, int64_t within) const;
    /// Where a camp is, by its number: east and north in world centimetres; empty if there is no such camp.
    godot::PackedInt64Array camp_at(int64_t camp) const;
    /// Whether the world is still catching up to where it was when it closed.
    bool catching_up() const;
    /// For the tests, while the world rests between batches, as after save_now: its whole state's digest.
    godot::String digest() const;

    // the benchmark's marks (A18.1)
    /// Takes the world's digest as it passes this game second, its whole state's for the crowd and its work's for the
    /// calendar's stand-in, as the cloud's headless run takes it (RES-05).
    void mark(int64_t second);
    /// The digests taken at the marks so far, in hexadecimal by game second.
    godot::Dictionary marks() const;
    /// Your command to call a camp home, by its number in id order, given as the world reaches exactly this game
    /// second and written to the journal as call_home's is.
    void call_home_at(int64_t camp, int64_t second);
    /// Asks the world to reach a moment without waiting for it, for a page whose frames have stopped asking.
    void reach(int64_t moment);
    /// A game second as the calendar says it, "Year 1, spring, day 1, 07:00" (TIM-14).
    static godot::String moment_text(int64_t second);
    /// The crowd's world as the Crowd page first makes it: its seed, and the moment the page opens on.
    static int64_t crowd_seed();
    static int64_t morning();

    /// The speed asked, in game seconds a real second, at least 1.
    void set_speed(double game_per_real);
    double speed() const;
    void pause();
    void play();
    bool is_paused() const;

    /// Once a frame: measures the real time since the last frame, moves the screen's time and sets the world's goal.
    void frame();

    /// The screen's game time, in game seconds.
    double screen_time() const;
    /// How far the world has got, in whole game seconds.
    int64_t frontier() const;
    /// The speed really drawn over the last real second, in game seconds a real second.
    double speed_shown() const;
    /// The screen's date, "Year 1, spring, day 1", and hour, "06:05" (TIM-14).
    godot::String date_text() const;
    godot::String time_text() const;

    /// The crowd's counters (PLT-01): events a second over the last real second, events, batches and the last
    /// batch's milliseconds, greetings, what the phone can do in game seconds a real second, the heat's working share
    /// and the speed limit it sets, the speeds asked and shown, the walkers, and how far the world is ahead; for a
    /// kept world, its saves, the pause the last one made the world take in milliseconds, whether a save failed and the
    /// world stopped, the real seconds it has run under this version, and the megabytes free where it is kept at the
    /// last save, and how few make the game warn.
    godot::Dictionary counters() const;
    /// The greetings since the last call, three numbers each: the game second, and the two walkers' ids.
    godot::PackedInt64Array drain_greetings();
    /// Pins the crowd's thread to these cores, or unpins it when empty, from its next batch (A3.9).
    void set_pinned(const godot::PackedInt32Array& cores);
    /// One reading of the phone's heat forecast, as Android's headroom, where 1 is severe throttling: returns the
    /// working share (A3.9).
    double heat_reading(double forecast);
    /// The phone's own light throttling threshold, as a headroom (Android 15 and later): time slows a margin below
    /// it rather than at the tuning file's near (A3.9).
    void set_heat_light(double threshold);
    /// Whether a game moment falls in the night, by the crowd's daylight.
    bool night_at(double t) const;
    /// The square the crowd keeps to: its west and south edges and its side, in world centimetres.
    godot::Dictionary crowd_square() const;
    /// Asks the world to reach a moment and waits until it has, for the tests.
    void run_until(int64_t moment);
    /// Before the first frame: runs the world to a moment and starts the screen's time there, as a page that opens
    /// on the morning does.
    void begin_at(int64_t moment);
    /// For the tests, while the world rests at its goal, as after run_until: each walker's place at the frontier by
    /// the world's own rule, in id order, as metres east and north of an origin in world centimetres.
    godot::PackedFloat64Array places(int64_t origin_east, int64_t origin_north) const;

    // for the crowd class
    [[nodiscard]] const DisplaySnapshot& display() const { return display_; }
    [[nodiscard]] CrowdStepper* stepper() const { return stepper_.get(); }
    [[nodiscard]] const data::Catalogue* catalogue() const { return catalogue_.get(); }
    [[nodiscard]] const demo::CrowdWorld* crowd() const { return crowd_.get(); }

protected:
    static void _bind_methods();

private:
    godot::Dictionary open_saved(const godot::String& folder, int64_t seed, int64_t camps, const godot::String& build,
                                 bool camp_alpha);
    DisplaySnapshot display_;
    [[nodiscard]] HeatRules heat_rules() const;
    /// The free space where the world is kept, in megabytes, as each save checks it (PLT-10).
    void check_space();
    /// On the world's thread: its snapshot handed to the keeper, timed.
    void snapshot();
    /// On the world's thread: your command to call a camp home, at the frontier, written to the journal.
    void called_home(ecs::Id camp);
    /// The runner, for a world and how to read its digest, with the marks between them.
    void start_runner(run::Steppable& world, std::function<std::uint64_t()> digest, time::Seconds start,
                      const char* thread);

    std::unique_ptr<data::Catalogue> catalogue_;
    std::unique_ptr<demo::Clockwork> clockwork_;
    // the folder a kept world is in, and its keeper, which outlast the world and its runner
    std::string folder_;
    std::unique_ptr<save::DiskFiles> files_;
    std::unique_ptr<save::Keeper> keeper_;
    std::unique_ptr<demo::CrowdWorld> crowd_;
    std::unique_ptr<CrowdStepper> stepper_;
    // the world as its runner steps it, with the benchmark's marks (A18.1)
    std::unique_ptr<run::Marked> marked_;
    std::unique_ptr<run::Runner> runner_;
    // the pause the last save made the world take, in milliseconds, written on the world's thread
    std::atomic<double> save_ms_{0.0};
    // catching up to where a reopened world had got, and the save that comes every so often
    time::Seconds catch_up_to_ = -1;
    double save_every_ = 30.0;
    double since_save_ = 0.0;
    // the megabytes free where the world is kept at the last save, and how few make the game warn (PLT-10); the
    // real time the world has run under this version, not yet told to the keeper (PLT-09)
    int64_t free_mb_ = -1;
    int64_t warn_below_mb_ = 1024;
    double played_ = 0.0;
    Pace pace_;
    TimeRequests time_requests_;
    bool time_requests_enabled_ = false;
    HeatGovernor heat_;
    std::chrono::steady_clock::time_point last_frame_;
    bool framed_ = false;
    // the crowd's events counted at each frame over the last real second: (real seconds since the start, events)
    std::deque<std::pair<double, std::uint64_t>> event_samples_;
    double real_ = 0.0;
};

}  // namespace kd::view
