// T2.9a.2: thin main-thread binding of the pure immutable streaming state (PLT-04, WLD-13).
#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <map>
#include "stream.hpp"
namespace kd::view {
class KdStream : public godot::RefCounted {
    GDCLASS(KdStream, godot::RefCounted)
public:
    godot::Dictionary begin(const godot::Dictionary& identity, const godot::Dictionary& limits);
    godot::Dictionary manifest(const godot::Dictionary& full);
    int64_t input_bytes_required(const godot::Dictionary& spec) const;
    godot::Dictionary request(const godot::Dictionary& spec);
    godot::Array take_jobs(int64_t count);
    godot::Dictionary ready(int64_t token, const godot::Dictionary& result);
    godot::Array take_ready(int64_t count);
    godot::Dictionary stage(int64_t token, const godot::String& channel, int64_t bytes);
    godot::Dictionary uploaded(int64_t token, const godot::String& channel, int64_t bytes, int64_t upload_us);
    godot::Dictionary publish(int64_t token);
    godot::Dictionary cancel(int64_t token);
    godot::Dictionary cancel_generation(int64_t generation);
    godot::Dictionary evict(const godot::Dictionary& key);
    godot::Dictionary reset();
    godot::Dictionary disposed(int64_t token, const godot::String& kind);
    godot::Dictionary release_cpu(int64_t token);
    godot::Dictionary reserve_target(int64_t bytes);
    godot::Dictionary release_target(int64_t bytes);
    godot::Dictionary status() const;

protected:
    static void _bind_methods();

private:
    godot::Dictionary reply(bool ok, StreamToken token = 0) const;
    StreamState state_;
    std::map<StreamToken, godot::Dictionary> specs_;
    std::map<StreamToken, godot::Dictionary> results_;
    std::uint64_t preparation_us_ = 0;
    std::uint64_t decode_us_ = 0;
    std::uint64_t upload_us_ = 0;
};
}  // namespace kd::view
