#include "stream.hpp"

#include <algorithm>
#include <set>
#include <utility>

namespace kd::view {
namespace {
bool fits(std::uint64_t used, std::uint64_t added, std::uint64_t cap) {
    return added <= cap && used <= cap - added;
}
std::uint64_t amount(const std::map<std::string, std::uint64_t>& values, const std::string& key) {
    const auto found = values.find(key);
    return found == values.end() ? 0 : found->second;
}
}  // namespace
void StreamState::peaks() {
    ledger_.peak_input = std::max(ledger_.peak_input, ledger_.input);
    ledger_.peak_prepared = std::max(ledger_.peak_prepared, ledger_.prepared);
    ledger_.peak_staging = std::max(ledger_.peak_staging, ledger_.staging);
    ledger_.peak_resident = std::max(ledger_.peak_resident, ledger_.resident);
    ledger_.peak_targets = std::max(ledger_.peak_targets, ledger_.targets);
}
void StreamState::begin(StreamIdentity identity, StreamLimits limits) {
    reset();
    identity_ = std::move(identity);
    limits_ = std::move(limits);
    manifest_ = {identity_.epoch, 0, 0.0, {}};
    problem_.clear();
}
bool StreamState::current(const StreamJob& job) const {
    if (job.phase == StreamPhase::retiring || job.request.identity != identity_ ||
        job.request.revision > manifest_.revision)
        return false;
    for (const auto& dep : job.request.dependencies) {
        const auto kind = manifest_.records.find(dep.kind);
        if (kind == manifest_.records.end()) return false;
        const auto record = kind->second.find(dep.id);
        if (record == kind->second.end() || record->second != dep.revision) return false;
    }
    return true;
}
bool StreamState::manifest(RevisionManifest full) {
    if (full.epoch != identity_.epoch || full.revision < manifest_.revision) {
        problem_ = "manifest epoch mismatch or revision moved backwards";
        return false;
    }
    manifest_ = std::move(full);
    for (auto& [token, job] : jobs_) {
        if (job.phase != StreamPhase::retiring && !current(job)) {
            ++stale_;
            retire(token, "complete manifest invalidated a dependency");
        }
    }
    return true;
}
bool StreamState::resident_fits(const StreamJob& job, std::uint64_t added) const {
    if (!fits(ledger_.resident, ledger_.reserved_resident, limits_.resident) ||
        !fits(ledger_.resident + ledger_.reserved_resident, added, limits_.resident))
        return false;
    const auto cap = limits_.resident_by_category.find(job.request.category);
    if (cap == limits_.resident_by_category.end()) return limits_.resident_by_category.empty();
    const auto resident = amount(ledger_.resident_by_category, job.request.category);
    const auto reserved = amount(ledger_.reserved_by_category, job.request.category);
    return fits(resident, reserved, cap->second) && fits(resident + reserved, added, cap->second);
}
StreamToken StreamState::request(StreamRequest input) {
    problem_.clear();
    StreamJob job;
    job.request = std::move(input);
    if (!current(job) || job.request.key.empty()) {
        ++stale_;
        problem_ = "request identity or referenced revision is stale";
        return 0;
    }
    std::uint64_t queued = 0;
    for (const auto& [token, existing] : jobs_) {
        if (existing.phase == StreamPhase::requested) ++queued;
        if (current(existing) && existing.request.key == job.request.key &&
            existing.request.generation == job.request.generation)
            return token;
        if (!job.request.parent.empty() && existing.request.key == job.request.parent &&
            existing.phase == StreamPhase::visible && current(existing))
            job.parent = token;
    }
    if ((!job.request.parent.empty() && job.parent == 0) || queued >= limits_.queued_jobs ||
        !fits(ledger_.input, job.request.input_bytes, limits_.input) ||
        !resident_fits(job, job.request.resident_bytes)) {
        problem_ = "request has no visible parent or lacks queue/input/resident headroom";
        return 0;
    }
    std::set<std::string> channels(job.request.channels.begin(), job.request.channels.end());
    if (channels.empty() || channels.size() != job.request.channels.size() ||
        job.request.metadata_bytes > job.request.input_bytes ||
        job.request.input.encoded.size() > job.request.input_bytes - job.request.metadata_bytes) {
        problem_ = "request channel bundle or encoded-byte reservation is invalid";
        return 0;
    }
    job.input_bytes = job.request.input_bytes;
    job.resident_reserved = job.request.resident_bytes;
    ledger_.input += job.input_bytes;
    ledger_.reserved_resident += job.resident_reserved;
    ledger_.reserved_by_category[job.request.category] += job.resident_reserved;
    if (job.parent != 0) ++jobs_.at(job.parent).pins;
    const auto token = ++next_;
    jobs_.emplace(token, std::move(job));
    peaks();
    return token;
}
std::vector<StreamWork> StreamState::take_jobs(std::uint64_t count) {
    std::vector<StreamWork> work;
    std::uint64_t running = 0;
    for (const auto& [token, job] : jobs_) {
        (void)token;
        if (job.worker_owned) ++running;
    }
    for (auto& [token, job] : jobs_) {
        if (work.size() >= count || running >= limits_.preparing_jobs) break;
        if (job.phase != StreamPhase::requested) continue;
        if (!current(job)) {
            ++stale_;
            retire(token, "stale before preparation");
            continue;
        }
        if (!fits(ledger_.prepared, job.request.prepared_bytes, limits_.prepared)) continue;
        job.prepared_bytes = job.request.prepared_bytes;
        ledger_.prepared += job.prepared_bytes;
        job.worker_owned = true;
        job.phase = StreamPhase::preparing;
        ++running;
        StreamRequest owned = job.request;
        owned.input = std::move(job.request.input);
        work.push_back({token, std::move(owned)});
    }
    peaks();
    return work;
}
bool StreamState::ready(StreamToken token, std::uint64_t actual_bytes, bool success) {
    const auto found = jobs_.find(token);
    if (found == jobs_.end()) return false;
    auto& job = found->second;
    if (!job.worker_owned) {
        problem_ = "completion has no owned preparation job";
        return false;
    }
    job.worker_owned = false;
    const bool headroom = actual_bytes <= job.prepared_bytes ||
                          fits(ledger_.prepared, actual_bytes - job.prepared_bytes, limits_.prepared);
    ledger_.prepared -= job.prepared_bytes;
    ledger_.prepared += actual_bytes;
    job.prepared_bytes = actual_bytes;
    peaks();
    if (!success || !headroom || !current(job)) {
        if (!success || !headroom)
            ++failed_;
        else
            ++stale_;
        retire(token, "completion failed, exceeded its reservation or became stale");
        return false;
    }
    job.phase = StreamPhase::ready;
    return true;
}
std::vector<StreamToken> StreamState::take_ready(std::uint64_t count) {
    std::vector<StreamToken> result;
    for (auto& [token, job] : jobs_) {
        if (result.size() >= count) break;
        if (job.phase == StreamPhase::ready && !job.ready_taken && current(job)) {
            job.ready_taken = true;
            result.push_back(token);
        }
    }
    return result;
}
bool StreamState::stage(StreamToken token, const std::string& channel, std::uint64_t bytes) {
    const auto found = jobs_.find(token);
    if (found == jobs_.end()) return false;
    auto& job = found->second;
    if (!current(job) || job.phase != StreamPhase::ready || !job.staged_channel.empty() ||
        job.uploaded.contains(channel) ||
        std::find(job.request.channels.begin(), job.request.channels.end(), channel) == job.request.channels.end() ||
        !fits(ledger_.staging, bytes, limits_.staging) ||
        !resident_fits(job, bytes > job.resident_reserved ? bytes - job.resident_reserved : 0)) {
        problem_ = "upload stage is stale, repeated or lacks measured headroom";
        return false;
    }
    job.staged_channel = channel;
    job.staging_bytes = bytes;
    ledger_.staging += bytes;
    peaks();
    return true;
}
bool StreamState::uploaded(StreamToken token, const std::string& channel, std::uint64_t bytes) {
    const auto found = jobs_.find(token);
    if (found == jobs_.end()) return false;
    auto& job = found->second;
    const bool fresh = current(job);
    if (job.staged_channel != channel || job.uploaded.contains(channel)) {
        problem_ = "uploaded channel has no matching owned stage";
        return false;
    }
    const bool headroom = resident_fits(job, bytes > job.resident_reserved ? bytes - job.resident_reserved : 0);
    ledger_.staging -= job.staging_bytes;
    job.staging_bytes = 0;
    job.staged_channel.clear();
    const auto used = std::min(bytes, job.resident_reserved);
    job.resident_reserved -= used;
    ledger_.reserved_resident -= used;
    ledger_.reserved_by_category[job.request.category] -= used;
    ledger_.resident += bytes;
    ledger_.resident_by_category[job.request.category] += bytes;
    job.uploaded[channel] = bytes;
    peaks();
    if (!fresh || !headroom) {
        if (!fresh)
            ++stale_;
        else
            ++failed_;
        retire(token, "late GPU allocation retained until actual disposal");
        return false;
    }
    if (job.uploaded.size() == job.request.channels.size()) job.phase = StreamPhase::uploaded;
    return true;
}
void StreamState::unpin(StreamJob& job) {
    if (job.parent != 0) {
        const auto parent = jobs_.find(job.parent);
        if (parent != jobs_.end() && parent->second.pins > 0) --parent->second.pins;
        job.parent = 0;
    }
}
bool StreamState::publish(StreamToken token) {
    const auto found = jobs_.find(token);
    if (found == jobs_.end()) return false;
    auto& job = found->second;
    if (!current(job) || job.phase != StreamPhase::uploaded || job.uploaded.size() != job.request.channels.size()) {
        problem_ = "only a complete current aligned bundle can be published";
        return false;
    }
    ledger_.reserved_resident -= job.resident_reserved;
    ledger_.reserved_by_category[job.request.category] -= job.resident_reserved;
    job.resident_reserved = 0;
    job.phase = StreamPhase::visible;
    unpin(job);
    return true;
}
bool StreamState::retire(StreamToken token, const std::string& why) {
    const auto found = jobs_.find(token);
    if (found == jobs_.end()) return false;
    auto& job = found->second;
    if (job.phase == StreamPhase::retiring) return true;
    unpin(job);
    const auto held = std::min(job.resident_reserved, job.staging_bytes);
    ledger_.reserved_resident -= job.resident_reserved - held;
    ledger_.reserved_by_category[job.request.category] -= job.resident_reserved - held;
    job.resident_reserved = held;
    job.phase = StreamPhase::retiring;
    job.reason = why;
    // Input, Images, staging and GPU bytes survive until their actual owner acknowledges disposal.
    return true;
}
bool StreamState::cancel(StreamToken token) {
    const auto found = jobs_.find(token);
    if (found == jobs_.end()) return false;
    if (found->second.phase != StreamPhase::retiring) ++cancelled_;
    return retire(token, "cancelled");
}
void StreamState::cancel_generation(std::uint64_t generation) {
    for (const auto& [token, job] : jobs_)
        if (job.request.generation == generation && job.phase != StreamPhase::visible) cancel(token);
}
bool StreamState::evict(const std::string& key) {
    for (const auto& [token, job] : jobs_) {
        if (job.request.key == key && job.phase != StreamPhase::retiring && job.request.identity == identity_) {
            if (job.pins != 0) {
                problem_ = "coarse parent is pinned by pending replacement";
                return false;
            }
            return retire(token, "evicted");
        }
    }
    return false;
}
void StreamState::reset() {
    for (const auto& [token, job] : jobs_) {
        (void)job;
        cancel(token);
    }
}
bool StreamState::disposed(StreamToken token, const std::string& kind) {
    const auto found = jobs_.find(token);
    if (found == jobs_.end()) return false;
    auto& job = found->second;
    if (job.worker_owned && (kind == "input" || kind == "prepared")) {
        problem_ = "preparation worker still owns the input and CPU reservation";
        return false;
    }
    if (kind == "input") {
        ledger_.input -= job.input_bytes - job.request.metadata_bytes;
        job.input_bytes = job.request.metadata_bytes;
        job.input_disposed = true;
        job.request.input = {};
    } else if (kind == "prepared") {
        ledger_.prepared -= job.prepared_bytes;
        job.prepared_bytes = 0;
    } else if (kind == "staging") {
        ledger_.staging -= job.staging_bytes;
        job.staging_bytes = 0;
        job.staged_channel.clear();
        if (job.phase == StreamPhase::retiring) {
            ledger_.reserved_resident -= job.resident_reserved;
            ledger_.reserved_by_category[job.request.category] -= job.resident_reserved;
            job.resident_reserved = 0;
        }
    } else if (kind == "gpu") {
        if (job.phase != StreamPhase::retiring) {
            problem_ = "visible GPU disposal requires retirement first";
            return false;
        }
        for (const auto& [channel, bytes] : job.uploaded) {
            (void)channel;
            ledger_.resident -= bytes;
            ledger_.resident_by_category[job.request.category] -= bytes;
        }
        job.uploaded.clear();
    } else
        return false;
    if (job.phase == StreamPhase::retiring && !job.worker_owned && job.input_disposed && job.prepared_bytes == 0 &&
        job.staging_bytes == 0 && job.uploaded.empty()) {
        ledger_.input -= job.input_bytes;
        jobs_.erase(found);
    }
    return true;
}
// T2.9a.2/PLT-04: bounded ticket metadata is charged too; reset cannot free another owner's resources.
StreamToken StreamState::reserve_allocation(StreamAllocation allocation) {
    constexpr std::uint64_t metadata = 256;
    const bool masks = allocation.category == "masks", targets = allocation.category == "targets";
    if ((!masks && !targets) || (masks && allocation.target != 0) ||
        (targets && (allocation.prepared != 0 || allocation.staging != 0 || allocation.resident != 0)) ||
        (allocation.prepared == 0 && allocation.staging == 0 && allocation.resident == 0 && allocation.target == 0) ||
        allocations_.size() >= 256) {
        problem_ = "allocation requires bounded masks/targets ownership and appropriate nonzero byte fields";
        return 0;
    }
    StreamJob check;
    check.request.category = allocation.category;
    if (!fits(ledger_.input, metadata, limits_.input) ||
        !fits(ledger_.prepared, allocation.prepared, limits_.prepared) ||
        !fits(ledger_.staging, allocation.staging, limits_.staging) ||
        !fits(ledger_.targets, allocation.target, limits_.targets) ||
        (allocation.resident != 0 && !resident_fits(check, allocation.resident))) {
        problem_ = "allocation lacks input/CPU/staging/resident/category/target headroom";
        return 0;
    }
    ledger_.input += metadata;
    ledger_.prepared += allocation.prepared;
    ledger_.staging += allocation.staging;
    ledger_.resident += allocation.resident;
    ledger_.resident_by_category[allocation.category] += allocation.resident;
    ledger_.targets += allocation.target;
    const auto token = ++next_;
    allocations_.emplace(token, std::move(allocation));
    peaks();
    problem_.clear();
    return token;
}
bool StreamState::release_allocation(StreamToken token) {
    const auto found = allocations_.find(token);
    if (found == allocations_.end()) {
        problem_ = "allocation ticket is not owned";
        return false;
    }
    const auto& allocation = found->second;
    ledger_.input -= 256;
    ledger_.prepared -= allocation.prepared;
    ledger_.staging -= allocation.staging;
    ledger_.resident -= allocation.resident;
    ledger_.resident_by_category[allocation.category] -= allocation.resident;
    ledger_.targets -= allocation.target;
    allocations_.erase(found);
    problem_.clear();
    return true;
}
bool StreamState::reserve_target(std::uint64_t bytes) {
    if (!fits(ledger_.targets, bytes, limits_.targets)) {
        problem_ = "target byte cap reached";
        return false;
    }
    ledger_.targets += bytes;
    manual_target_bytes_ += bytes;
    peaks();
    return true;
}
bool StreamState::release_target(std::uint64_t bytes) {
    if (bytes > manual_target_bytes_) return false;
    manual_target_bytes_ -= bytes;
    ledger_.targets -= bytes;
    return true;
}
const StreamJob* StreamState::job(StreamToken token) const {
    const auto found = jobs_.find(token);
    return found == jobs_.end() ? nullptr : &found->second;
}
}  // namespace kd::view
