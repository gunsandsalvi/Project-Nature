// T2.9a.2: immutable jobs, complete revisions and conservative byte ownership (A4.6, A18.1).
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace kd::view {
using StreamToken = std::uint64_t;
struct StreamIdentity {
    std::string world;
    std::string data;
    std::string look;
    std::string renderer;
    std::uint64_t format = 1;
    std::uint64_t epoch = 0;
    bool operator==(const StreamIdentity&) const = default;
};
struct RevisionDependency {
    std::string kind;
    std::string id;
    std::uint64_t revision = 0;
};
struct RevisionManifest {
    std::uint64_t epoch = 0;
    std::uint64_t revision = 0;
    double second = 0.0;
    std::map<std::string, std::map<std::string, std::uint64_t>> records;
};
struct StreamLimits {
    std::uint64_t queued_jobs = 32;
    std::uint64_t preparing_jobs = 1;
    std::uint64_t input = 8 * 1024 * 1024;
    std::uint64_t prepared = 64 * 1024 * 1024;
    std::uint64_t staging = 4 * 1024 * 1024;
    std::uint64_t resident = 128 * 1024 * 1024;
    std::uint64_t targets = 32 * 1024 * 1024;
    std::map<std::string, std::uint64_t> resident_by_category;
};
// T2.9a.2: main-owned allocations use the same caps without borrowing a mutable scene for worker preparation.
struct StreamAllocation {
    std::string category;
    std::uint64_t prepared = 0, staging = 0, resident = 0, target = 0;
};
struct StreamLedger {
    std::uint64_t input = 0;
    std::uint64_t prepared = 0;
    std::uint64_t staging = 0;
    std::uint64_t resident = 0;
    std::uint64_t reserved_resident = 0;
    std::uint64_t targets = 0;
    std::uint64_t peak_input = 0;
    std::uint64_t peak_prepared = 0;
    std::uint64_t peak_staging = 0;
    std::uint64_t peak_resident = 0;
    std::uint64_t peak_targets = 0;
    std::map<std::string, std::uint64_t> resident_by_category;
    std::map<std::string, std::uint64_t> reserved_by_category;
};
struct OwnedStreamInput {
    std::vector<std::uint8_t> encoded;
    std::string metadata;
};
struct StreamRequest {
    StreamIdentity identity;
    std::string key;
    std::string parent;
    std::string category = "sprites";
    std::uint64_t generation = 0;
    std::uint64_t revision = 0;
    std::uint64_t input_bytes = 0;
    std::uint64_t metadata_bytes = 0;
    std::uint64_t prepared_width = 0;
    std::uint64_t prepared_height = 0;
    std::uint64_t prepared_bytes = 0;
    std::uint64_t resident_bytes = 0;
    std::vector<std::string> channels{"colour", "normal", "material"};
    std::vector<RevisionDependency> dependencies;
    OwnedStreamInput input;
};
struct StreamWork {
    StreamToken token = 0;
    StreamRequest request;
};
enum class StreamPhase : std::uint8_t { requested, preparing, ready, uploaded, visible, retiring };
struct StreamJob {
    StreamRequest request;
    StreamPhase phase = StreamPhase::requested;
    std::uint64_t input_bytes = 0;
    std::uint64_t prepared_bytes = 0;
    std::uint64_t resident_reserved = 0;
    std::uint64_t staging_bytes = 0;
    std::map<std::string, std::uint64_t> uploaded;
    std::string staged_channel;
    StreamToken parent = 0;
    std::uint64_t pins = 0;
    bool input_disposed = false;
    bool ready_taken = false;
    bool worker_owned = false;
    std::string reason;
};

/// Implements PLT-04/WLD-13/PRE-03: callers own actual data; this tracks reservations until disposal acknowledgement.
class StreamState {
public:
    void begin(StreamIdentity identity, StreamLimits limits);
    bool manifest(RevisionManifest full);
    StreamToken request(StreamRequest input);
    std::vector<StreamWork> take_jobs(std::uint64_t count);
    bool ready(StreamToken token, std::uint64_t actual_bytes, bool success = true);
    std::vector<StreamToken> take_ready(std::uint64_t count);
    bool stage(StreamToken token, const std::string& channel, std::uint64_t bytes);
    bool uploaded(StreamToken token, const std::string& channel, std::uint64_t bytes);
    bool publish(StreamToken token);
    bool cancel(StreamToken token);
    void cancel_generation(std::uint64_t generation);
    bool evict(const std::string& key);
    void reset();
    bool disposed(StreamToken token, const std::string& kind);
    StreamToken reserve_allocation(StreamAllocation allocation);
    bool release_allocation(StreamToken token);
    [[nodiscard]] const std::map<StreamToken, StreamAllocation>& allocations() const { return allocations_; }
    bool reserve_target(std::uint64_t bytes);
    bool release_target(std::uint64_t bytes);
    [[nodiscard]] const StreamJob* job(StreamToken token) const;
    [[nodiscard]] const StreamLedger& ledger() const { return ledger_; }
    [[nodiscard]] const StreamIdentity& identity() const { return identity_; }
    [[nodiscard]] const RevisionManifest& revisions() const { return manifest_; }
    [[nodiscard]] const std::map<StreamToken, StreamJob>& jobs() const { return jobs_; }
    [[nodiscard]] const std::string& problem() const { return problem_; }
    [[nodiscard]] std::uint64_t cancelled() const { return cancelled_; }
    [[nodiscard]] std::uint64_t stale() const { return stale_; }
    [[nodiscard]] std::uint64_t failed() const { return failed_; }

private:
    bool current(const StreamJob& job) const;
    bool retire(StreamToken token, const std::string& why);
    bool resident_fits(const StreamJob& job, std::uint64_t added) const;
    void peaks();
    void unpin(StreamJob& job);
    StreamIdentity identity_;
    StreamLimits limits_;
    RevisionManifest manifest_;
    StreamLedger ledger_;
    std::map<StreamToken, StreamJob> jobs_;
    std::map<StreamToken, StreamAllocation> allocations_;
    StreamToken next_ = 0;
    std::uint64_t manual_target_bytes_ = 0;
    std::string problem_;
    std::uint64_t cancelled_ = 0;
    std::uint64_t stale_ = 0;
    std::uint64_t failed_ = 0;
};
}  // namespace kd::view
