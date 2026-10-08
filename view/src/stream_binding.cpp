#include "stream_binding.hpp"
#include <algorithm>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_float64_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_int64_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_vector4_array.hpp>

namespace kd::view {
namespace {
std::string text(const godot::Variant& v) {
    return godot::String(v).utf8().get_data();
}
godot::String string(const std::string& value) {
    return godot::String::utf8(value.c_str());
}
std::uint64_t whole(const godot::Dictionary& d, const char* key, std::uint64_t fallback = 0) {
    const auto v = d.get(key, static_cast<int64_t>(fallback));
    return v.get_type() == godot::Variant::INT && static_cast<int64_t>(v) >= 0
               ? static_cast<std::uint64_t>(static_cast<int64_t>(v))
               : 0;
}
StreamIdentity identity_of(const godot::Dictionary& d, std::uint64_t epoch) {
    return {text(d.get("world_id", "")), text(d.get("data_hash", "")), text(d.get("look_hash", "")),
            text(d.get("renderer", "")), whole(d, "format_version"),   epoch};
}
const char* phase(StreamPhase value) {
    constexpr const char* names[] = {"requested", "preparing", "ready", "uploaded", "visible", "retiring"};
    return names[static_cast<std::size_t>(value)];
}
godot::Dictionary error(const char* message) {
    godot::Dictionary result;
    result["ok"] = false;
    result["problem"] = message;
    return result;
}
bool plain(const godot::Variant& value, int depth = 0) {
    if (depth > 16 || value.get_type() == godot::Variant::OBJECT || value.get_type() == godot::Variant::CALLABLE ||
        value.get_type() == godot::Variant::SIGNAL || value.get_type() == godot::Variant::RID)
        return false;
    if (value.get_type() == godot::Variant::DICTIONARY) {
        const godot::Dictionary d = value;
        for (const auto& key : d.keys())
            if (!plain(key, depth + 1) || !plain(d[key], depth + 1)) return false;
    } else if (value.get_type() == godot::Variant::ARRAY) {
        const godot::Array a = value;
        for (const auto& item : a)
            if (!plain(item, depth + 1)) return false;
    }
    return true;
}
// Conservative owned payload/container accounting before a deep copy. Empty inputs cost no payload.
bool input_size(const godot::Variant& value, std::uint64_t limit, std::uint64_t& used, std::uint64_t& nodes,
                int depth = 0) {
    if (++nodes > 65536 || depth > 16) return false;
    const auto add = [&](std::uint64_t bytes) {
        if (bytes > limit || used > limit - bytes) return false;
        used += bytes;
        return true;
    };
    using godot::Variant;
    switch (value.get_type()) {
        case Variant::STRING:
        case Variant::STRING_NAME:
        case Variant::NODE_PATH: {
            const godot::String s = value;
            return add(32 + 4 * static_cast<std::uint64_t>(s.length() + 1));
        }
        case Variant::PACKED_BYTE_ARRAY: {
            const godot::PackedByteArray bytes = value;
            return bytes.is_empty() || add(32 + static_cast<std::uint64_t>(bytes.size()));
        }
        case Variant::ARRAY: {
            const godot::Array a = value;
            if (!a.is_empty() && !add(32 + 32 * static_cast<std::uint64_t>(a.size()))) return false;
            for (const auto& item : a)
                if (!input_size(item, limit, used, nodes, depth + 1)) return false;
            return true;
        }
        case Variant::DICTIONARY: {
            const godot::Dictionary d = value;
            if (!d.is_empty() && !add(64 + 128 * static_cast<std::uint64_t>(d.size()))) return false;
            for (const auto& key : d.keys())
                if (!input_size(key, limit, used, nodes, depth + 1) ||
                    !input_size(d[key], limit, used, nodes, depth + 1))
                    return false;
            return true;
        }
        case Variant::PACKED_INT32_ARRAY:
            return add(32 + 4 * static_cast<std::uint64_t>(godot::PackedInt32Array(value).size()));
        case Variant::PACKED_INT64_ARRAY:
            return add(32 + 8 * static_cast<std::uint64_t>(godot::PackedInt64Array(value).size()));
        case Variant::PACKED_FLOAT32_ARRAY:
            return add(32 + 4 * static_cast<std::uint64_t>(godot::PackedFloat32Array(value).size()));
        case Variant::PACKED_FLOAT64_ARRAY:
            return add(32 + 8 * static_cast<std::uint64_t>(godot::PackedFloat64Array(value).size()));
        case Variant::PACKED_VECTOR2_ARRAY:
            return add(32 +
                       sizeof(godot::Vector2) * static_cast<std::uint64_t>(godot::PackedVector2Array(value).size()));
        case Variant::PACKED_VECTOR3_ARRAY:
            return add(32 +
                       sizeof(godot::Vector3) * static_cast<std::uint64_t>(godot::PackedVector3Array(value).size()));
        case Variant::PACKED_VECTOR4_ARRAY:
            return add(32 +
                       sizeof(godot::Vector4) * static_cast<std::uint64_t>(godot::PackedVector4Array(value).size()));
        case Variant::TRANSFORM2D:
        case Variant::TRANSFORM3D:
        case Variant::BASIS:
        case Variant::AABB:
        case Variant::PROJECTION:
            return add(128);
        case Variant::PACKED_COLOR_ARRAY:
            return add(32 + sizeof(godot::Color) * static_cast<std::uint64_t>(godot::PackedColorArray(value).size()));
        case Variant::PACKED_STRING_ARRAY: {
            const godot::PackedStringArray strings = value;
            if (!add(32 + 8 * static_cast<std::uint64_t>(strings.size()))) return false;
            for (const auto& item : strings)
                if (!input_size(item, limit, used, nodes, depth + 1)) return false;
            return true;
        }
        default:
            return plain(value);
    }
}
std::string key_of(const godot::Dictionary& key) {
    std::vector<std::pair<std::string, std::string>> fields;
    if (key.size() > 64) return {};
    for (const auto& name : key.keys()) {
        if (name.get_type() != godot::Variant::STRING) return {};
        const auto field = text(name);
        if (field.size() > 64) return {};
        if (field == "epoch" || field == "generation" || field == "revision") continue;
        const auto value = key[name];
        if (value.get_type() != godot::Variant::STRING && value.get_type() != godot::Variant::INT) return {};
        if (value.get_type() == godot::Variant::STRING && godot::String(value).length() > 512) return {};
        fields.emplace_back(field, std::to_string(static_cast<int>(value.get_type())) + ":" + text(value));
    }
    std::sort(fields.begin(), fields.end());
    std::string result;
    for (const auto& [name, value] : fields)
        result += std::to_string(name.size()) + ":" + name + std::to_string(value.size()) + ":" + value;
    return result;
}
}  // namespace
void KdStream::_bind_methods() {
    using namespace godot;
    ClassDB::bind_method(D_METHOD("begin", "identity", "limits"), &KdStream::begin);
    ClassDB::bind_method(D_METHOD("manifest", "full"), &KdStream::manifest);
    ClassDB::bind_method(D_METHOD("input_bytes_required", "spec"), &KdStream::input_bytes_required);
    ClassDB::bind_method(D_METHOD("request", "spec"), &KdStream::request);
    ClassDB::bind_method(D_METHOD("take_jobs", "max_count"), &KdStream::take_jobs);
    ClassDB::bind_method(D_METHOD("ready", "token", "result"), &KdStream::ready);
    ClassDB::bind_method(D_METHOD("take_ready", "max_count"), &KdStream::take_ready);
    ClassDB::bind_method(D_METHOD("stage", "token", "channel", "bytes"), &KdStream::stage);
    ClassDB::bind_method(D_METHOD("uploaded", "token", "channel", "resident_bytes", "upload_us"), &KdStream::uploaded);
    ClassDB::bind_method(D_METHOD("publish", "token"), &KdStream::publish);
    ClassDB::bind_method(D_METHOD("cancel", "token"), &KdStream::cancel);
    ClassDB::bind_method(D_METHOD("cancel_generation", "generation"), &KdStream::cancel_generation);
    ClassDB::bind_method(D_METHOD("evict", "key"), &KdStream::evict);
    ClassDB::bind_method(D_METHOD("reset"), &KdStream::reset);
    ClassDB::bind_method(D_METHOD("disposed", "token", "kind"), &KdStream::disposed);
    ClassDB::bind_method(D_METHOD("release_cpu", "token"), &KdStream::release_cpu);
    ClassDB::bind_method(D_METHOD("reserve_target", "bytes"), &KdStream::reserve_target);
    ClassDB::bind_method(D_METHOD("release_target", "bytes"), &KdStream::release_target);
    ClassDB::bind_method(D_METHOD("status"), &KdStream::status);
}
godot::Dictionary KdStream::reply(bool ok, StreamToken token) const {
    godot::Dictionary result;
    result["ok"] = ok;
    result["token"] = static_cast<int64_t>(token);
    result["problem"] = ok ? godot::String() : string(state_.problem());
    const auto* job = state_.job(token);
    result["state"] = job == nullptr ? godot::String() : godot::String(phase(job->phase));
    return result;
}
godot::Dictionary KdStream::begin(const godot::Dictionary& id, const godot::Dictionary& values) {
    auto identity = identity_of(id, whole(id, "epoch"));
    if (identity.world.empty() || identity.data.empty() || identity.look.empty() || identity.renderer.empty() ||
        identity.format == 0 || identity.epoch == 0)
        return error("begin requires complete world/data/look/renderer/format/epoch identity");
    StreamLimits limits;
    limits.queued_jobs = whole(values, "queued_jobs", limits.queued_jobs);
    limits.preparing_jobs = whole(values, "preparing_jobs", limits.preparing_jobs);
    limits.input = whole(values, "input_bytes", limits.input);
    limits.prepared = whole(values, "prepared_bytes", limits.prepared);
    limits.staging = whole(values, "staging_bytes", limits.staging);
    limits.resident = whole(values, "resident_bytes", limits.resident);
    limits.targets = whole(values, "target_bytes", limits.targets);
    if (limits.queued_jobs == 0 || limits.preparing_jobs == 0 || limits.input == 0 || limits.prepared == 0 ||
        limits.staging == 0 || limits.resident == 0 || limits.targets == 0)
        return error("stream limits must be positive whole counts and bytes");
    const godot::Dictionary categories = values.get("resident_by_category", godot::Dictionary());
    for (const auto& name : categories.keys()) {
        const auto value = categories[name];
        if (value.get_type() != godot::Variant::INT || static_cast<int64_t>(value) <= 0)
            return error("resident category caps must be positive whole bytes");
        limits.resident_by_category[text(name)] = static_cast<std::uint64_t>(static_cast<int64_t>(value));
    }
    state_.begin(std::move(identity), std::move(limits));
    return reply(true);
}
godot::Dictionary KdStream::manifest(const godot::Dictionary& full) {
    RevisionManifest manifest;
    manifest.epoch = whole(full, "epoch");
    manifest.revision = whole(full, "revision");
    manifest.second = full.get("second", 0.0);
    for (const auto& [plural, singular] : std::vector<std::pair<const char*, const char*>>{
             {"surfaces", "surface"}, {"casters", "caster"}, {"appearances", "appearance"}}) {
        const godot::Dictionary records = full.get(plural, godot::Dictionary());
        for (const auto& name : records.keys()) {
            const auto value = records[name];
            if (value.get_type() != godot::Variant::INT || static_cast<int64_t>(value) < 0)
                return error("manifest revisions must be nonnegative whole numbers");
            manifest.records[singular][text(name)] = static_cast<std::uint64_t>(static_cast<int64_t>(value));
        }
    }
    return reply(state_.manifest(std::move(manifest)));
}
// T2.9a.2: charge all copied request metadata, including queue/worker overlap and native key/dependency copies.
int64_t KdStream::input_bytes_required(const godot::Dictionary& spec) const {
    std::uint64_t payload = 0, nodes = 0;
    constexpr std::uint64_t maximum = 8ULL * 1024 * 1024 * 1024;
    if (!input_size(spec, maximum / 4, payload, nodes) || !plain(spec)) return -1;
    return static_cast<int64_t>(payload * 4);
}
godot::Dictionary KdStream::request(const godot::Dictionary& spec) {
    const auto required = input_bytes_required(spec);
    if (required < 0) return error("jobs require bounded owned plain data, never objects, handles or callables");
    const godot::Dictionary key = spec.get("key", godot::Dictionary());
    const godot::Dictionary parent = spec.get("parent_key", godot::Dictionary());
    const godot::Dictionary reserve = spec.get("reserve", godot::Dictionary());
    if (static_cast<std::uint64_t>(required) > whole(reserve, "input"))
        return error("owned job metadata and input exceed their byte reservation");
    StreamRequest request;
    request.identity = identity_of(key, whole(spec, "epoch", state_.identity().epoch));
    request.key = key_of(key);
    request.parent = parent.is_empty() ? "" : key_of(parent);
    if (!parent.is_empty() && request.parent.empty())
        return error("parent key is not an immutable integer/string identity");
    request.generation = whole(spec, "generation");
    request.revision = whole(spec, "revision");
    request.category = text(reserve.get("category", "sprites"));
    request.input_bytes = whole(reserve, "input");
    godot::Dictionary metadata = spec.duplicate(false);
    metadata.erase("input");
    const auto retained = input_bytes_required(metadata);
    request.metadata_bytes = retained < 0 ? request.input_bytes : static_cast<std::uint64_t>(retained);
    request.prepared_bytes = whole(reserve, "prepared");
    request.resident_bytes = whole(reserve, "resident");
    const godot::Array deps = spec.get("dependencies", godot::Array());
    for (const auto& value : deps) {
        const godot::Dictionary dependency = value;
        request.dependencies.push_back(
            {text(dependency.get("kind", "")), text(dependency.get("id", "")), whole(dependency, "revision")});
    }
    if (spec.has("channels")) {
        request.channels.clear();
        const godot::Array channels = spec["channels"];
        for (const auto& channel : channels) request.channels.push_back(text(channel));
    }
    const auto token = state_.request(std::move(request));
    if (token != 0 && !specs_.contains(token)) specs_[token] = spec.duplicate(true);
    return reply(token != 0, token);
}
godot::Array KdStream::take_jobs(int64_t count) {
    godot::Array rows;
    if (count <= 0) return rows;
    for (const auto& work : state_.take_jobs(static_cast<std::uint64_t>(count))) {
        auto& saved = specs_.at(work.token);
        godot::Dictionary row = saved.duplicate(true);
        saved.erase("input");
        row["token"] = static_cast<int64_t>(work.token);
        row["epoch"] = static_cast<int64_t>(work.request.identity.epoch);
        rows.push_back(row);
    }
    return rows;
}
godot::Dictionary KdStream::ready(int64_t token, const godot::Dictionary& result) {
    const auto native = static_cast<StreamToken>(token);
    const auto* job = state_.job(native);
    if (token <= 0 || job == nullptr || !job->worker_owned)
        return error("completion token has no owned preparation job");
    const godot::Dictionary key = result.get("key", godot::Dictionary());
    const bool matches = key_of(key) == job->request.key && whole(result, "epoch") == job->request.identity.epoch &&
                         whole(result, "generation") == job->request.generation &&
                         whole(result, "revision") == job->request.revision;
    const godot::Dictionary images = result.get("images", godot::Dictionary());
    std::uint64_t actual = 0;
    bool aligned = images.size() == static_cast<int64_t>(job->request.channels.size());
    for (const auto& channel : images.keys()) {
        const godot::Ref<godot::Image> image = images[channel];
        if (image.is_null()) {
            aligned = false;
            continue;
        }
        actual += static_cast<std::uint64_t>(std::max<int64_t>(0, image->get_data_size()));
        aligned = aligned && !image->is_empty() && !image->has_mipmaps() &&
                  image->get_format() == godot::Image::FORMAT_RGBA8 &&
                  std::find(job->request.channels.begin(), job->request.channels.end(), text(channel)) !=
                      job->request.channels.end();
    }
    godot::Dictionary metadata = result.duplicate(false);
    metadata.erase("images");
    const auto metadata_bytes = input_bytes_required(metadata);
    const bool bounded =
        metadata_bytes >= 0 && static_cast<std::uint64_t>(metadata_bytes) <= job->request.metadata_bytes;
    // Images are private worker-owned resources. Inspect their existing byte size without allocating a data copy.
    godot::Dictionary retained;
    if (bounded) retained = metadata.duplicate(true);
    retained["images"] = images.duplicate(false);
    retained["cpu_bytes"] = static_cast<int64_t>(actual);
    results_[native] = retained;
    preparation_us_ += whole(result, "preparation_us");
    decode_us_ += whole(result, "decode_us");
    const auto success = matches && bounded && aligned && actual == whole(result, "cpu_bytes") &&
                         text(result.get("problem", "")).empty();
    const auto accepted = state_.ready(native, actual, success);
    return reply(accepted, native);
}
godot::Array KdStream::take_ready(int64_t count) {
    godot::Array rows;
    if (count <= 0) return rows;
    for (const auto token : state_.take_ready(static_cast<std::uint64_t>(count))) {
        auto found = results_.find(token);
        if (found == results_.end()) continue;
        godot::Dictionary row = found->second;
        row["token"] = static_cast<int64_t>(token);
        rows.push_back(row);
        results_.erase(found);
    }
    return rows;
}
godot::Dictionary KdStream::stage(int64_t token, const godot::String& channel, int64_t bytes) {
    return reply(token > 0 && bytes >= 0 &&
                     state_.stage(static_cast<StreamToken>(token), text(channel), static_cast<std::uint64_t>(bytes)),
                 static_cast<StreamToken>(token));
}
godot::Dictionary KdStream::uploaded(int64_t token, const godot::String& channel, int64_t bytes, int64_t upload_us) {
    if (upload_us > 0) upload_us_ += static_cast<std::uint64_t>(upload_us);
    return reply(token > 0 && bytes >= 0 &&
                     state_.uploaded(static_cast<StreamToken>(token), text(channel), static_cast<std::uint64_t>(bytes)),
                 static_cast<StreamToken>(token));
}
godot::Dictionary KdStream::publish(int64_t token) {
    return reply(token > 0 && state_.publish(static_cast<StreamToken>(token)), static_cast<StreamToken>(token));
}
godot::Dictionary KdStream::cancel(int64_t token) {
    return reply(token > 0 && state_.cancel(static_cast<StreamToken>(token)), static_cast<StreamToken>(token));
}
godot::Dictionary KdStream::cancel_generation(int64_t generation) {
    if (generation < 0) return error("negative generation");
    state_.cancel_generation(static_cast<std::uint64_t>(generation));
    return reply(true);
}
godot::Dictionary KdStream::evict(const godot::Dictionary& key) {
    return reply(state_.evict(key_of(key)));
}
godot::Dictionary KdStream::reset() {
    state_.reset();
    return reply(true);
}
godot::Dictionary KdStream::disposed(int64_t token, const godot::String& kind) {
    const auto native = static_cast<StreamToken>(token);
    if (kind == "input") {
        const auto found = specs_.find(native);
        if (found != specs_.end()) found->second.erase("input");
    }
    if (kind == "prepared") results_.erase(native);
    const auto ok = token > 0 && state_.disposed(native, text(kind));
    if (state_.job(native) == nullptr) {
        specs_.erase(native);
        results_.erase(native);
    }
    return reply(ok, native);
}
godot::Dictionary KdStream::release_cpu(int64_t token) {
    const auto prepared = disposed(token, "prepared");
    const auto input = disposed(token, "input");
    return reply(static_cast<bool>(prepared.get("ok", false)) && static_cast<bool>(input.get("ok", false)),
                 static_cast<StreamToken>(token));
}
godot::Dictionary KdStream::reserve_target(int64_t bytes) {
    return reply(bytes >= 0 && state_.reserve_target(static_cast<std::uint64_t>(bytes)));
}
godot::Dictionary KdStream::release_target(int64_t bytes) {
    return reply(bytes >= 0 && state_.release_target(static_cast<std::uint64_t>(bytes)));
}
godot::Dictionary KdStream::status() const {
    godot::Dictionary result;
    const auto& ledger = state_.ledger();
    result["input_bytes"] = static_cast<int64_t>(ledger.input);
    result["prepared_bytes"] = static_cast<int64_t>(ledger.prepared);
    result["staging_bytes"] = static_cast<int64_t>(ledger.staging);
    result["resident_bytes"] = static_cast<int64_t>(ledger.resident);
    result["reserved_resident_bytes"] = static_cast<int64_t>(ledger.reserved_resident);
    result["target_bytes"] = static_cast<int64_t>(ledger.targets);
    result["peak_input_bytes"] = static_cast<int64_t>(ledger.peak_input);
    result["peak_prepared_bytes"] = static_cast<int64_t>(ledger.peak_prepared);
    result["peak_staging_bytes"] = static_cast<int64_t>(ledger.peak_staging);
    result["peak_resident_bytes"] = static_cast<int64_t>(ledger.peak_resident);
    result["peak_target_bytes"] = static_cast<int64_t>(ledger.peak_targets);
    godot::Dictionary categories;
    for (const auto& [name, bytes] : ledger.resident_by_category)
        categories[string(name)] = static_cast<int64_t>(bytes);
    result["resident_by_category"] = categories;
    godot::Dictionary reserved;
    for (const auto& [name, bytes] : ledger.reserved_by_category) reserved[string(name)] = static_cast<int64_t>(bytes);
    result["reserved_by_category"] = reserved;
    for (const auto* name : {"requested", "preparing", "ready", "uploaded", "visible", "retiring"}) result[name] = 0;
    godot::Array jobs;
    for (const auto& [token, job] : state_.jobs()) {
        const auto name = godot::String(phase(job.phase));
        result[name] = static_cast<int64_t>(result[name]) + 1;
        godot::Dictionary row;
        row["token"] = static_cast<int64_t>(token);
        row["state"] = name;
        row["worker_owned"] = job.worker_owned;
        row["input_bytes"] = static_cast<int64_t>(job.input_bytes);
        row["metadata_bytes"] = static_cast<int64_t>(job.request.metadata_bytes);
        row["prepared_bytes"] = static_cast<int64_t>(job.prepared_bytes);
        row["staging_bytes"] = static_cast<int64_t>(job.staging_bytes);
        row["reason"] = string(job.reason);
        row["category"] = string(job.request.category);
        const auto saved = specs_.find(token);
        if (saved != specs_.end())
            row["key"] = godot::Dictionary(saved->second.get("key", godot::Dictionary())).duplicate(true);
        jobs.push_back(row);
    }
    result["jobs"] = jobs;
    result["epoch"] = static_cast<int64_t>(state_.identity().epoch);
    result["revision"] = static_cast<int64_t>(state_.revisions().revision);
    result["second"] = state_.revisions().second;
    result["cancelled"] = static_cast<int64_t>(state_.cancelled());
    result["stale"] = static_cast<int64_t>(state_.stale());
    result["failed"] = static_cast<int64_t>(state_.failed());
    result["preparation_us"] = static_cast<int64_t>(preparation_us_);
    result["decode_us"] = static_cast<int64_t>(decode_us_);
    result["upload_us"] = static_cast<int64_t>(upload_us_);
    result["problem"] = string(state_.problem());
    return result;
}
}  // namespace kd::view
