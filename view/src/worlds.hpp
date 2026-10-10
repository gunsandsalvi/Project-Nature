// The worlds kept on the phone (A3.7): each a folder under one root, user://worlds/<id>/, and a file naming the one
// the Crowd page opens. The Worlds page lists them with their names, moments and sizes by part, makes, renames and
// deletes them, and moves one in or out as a .kindling file through Android's file picker, a piece at a time, so a
// world of any size passes without being held whole. It never deletes a world itself (PLT-10).
#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include "kd/save/archive.hpp"
#include "kd/save/files.hpp"

namespace kd::view {

/// Implements TIM-08, PLT-08, PLT-10, PLT-05 and RES-10, see A3.7: the worlds on the phone, a test's marked as one.
class KdWorlds : public godot::RefCounted {
    GDCLASS(KdWorlds, godot::RefCounted)

public:
    /// The folder the worlds are in, an absolute path; made if it is missing.
    void set_root(const godot::String& root);

    /// Every world, by name: its id, its name (empty when it has none, as α1.4a's), seed, whether it is a test's
    /// world and the switches it ran with, the moment its newest snapshot is at and its words, when it was saved in
    /// seconds since 1970, the version that saved it, and its size in bytes in all and by part: snapshots, journal,
    /// history and the copy kept from before an update.
    godot::Array list() const;
    /// A new world's folder with its world.toml, its name, seed and camps (0 for the tuning's): its id. The world is
    /// made as it is first opened.
    godot::String make_discovery(const godot::String& name, int64_t seed);
    godot::String make_cold_discovery(const godot::String& name, int64_t seed);
    /// Gives a world a new name; false if it has no world.toml that can be read.
    bool rename(const godot::String& id, const godot::String& name);
    /// Deletes a world's folder and everything in it, as you asked twice.
    bool remove(const godot::String& id);
    /// The world the Crowd page opens, and setting it.
    godot::String current() const;
    void set_current(const godot::String& id);
    /// The space free where the worlds are, in bytes; -1 if it cannot be told.
    int64_t free_space() const;

    /// Begins to write a world out as one .kindling file: about how many bytes it takes, its parts' sizes, or -1 if
    /// there is no such world.
    int64_t export_begin(const godot::String& id);
    /// The file's next piece, at most this many bytes; empty once it is whole.
    godot::PackedByteArray export_next(int64_t most);
    /// Begins to read a .kindling file, into a hidden folder until it is whole; false if that cannot be made.
    bool import_begin();
    /// The file's next piece, checked as it comes; false once something is wrong.
    bool import_feed(const godot::PackedByteArray& piece);
    /// After the last piece: the new world's id and name, or why it was refused, in which case nothing is left.
    godot::Dictionary import_finish();

protected:
    static void _bind_methods();

private:
    godot::String make_discovery_state(const godot::String& name, int64_t seed, bool fire_already_out);
    [[nodiscard]] std::string folder(const godot::String& id) const;
    [[nodiscard]] std::string new_id() const;

    std::string root_;
    std::unique_ptr<save::DiskFiles> export_files_;
    std::unique_ptr<save::ArchiveWriter> writer_;
    std::unique_ptr<save::DiskFiles> import_files_;
    std::unique_ptr<save::ArchiveReader> reader_;
};

}  // namespace kd::view
