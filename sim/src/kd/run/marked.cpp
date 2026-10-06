#include "kd/run/marked.hpp"

#include <algorithm>
#include <utility>
#include <vector>

#include "kd/core/check.hpp"

namespace kd::run {

Marked::Marked(Steppable& world, std::function<std::uint64_t()> digest) : world_(world), digest_(std::move(digest)) {}

void Marked::mark(time::Seconds at) {
    std::lock_guard lock(mutex_);
    marks_.insert(at);
}

void Marked::at(time::Seconds second, std::function<void()> job) {
    std::lock_guard lock(mutex_);
    jobs_.emplace(second, std::move(job));
}

std::optional<std::uint64_t> Marked::digest_at(time::Seconds mark) const {
    std::lock_guard lock(mutex_);
    const auto it = taken_.find(mark);
    return it == taken_.end() ? std::nullopt : std::optional<std::uint64_t>(it->second);
}

std::map<time::Seconds, std::uint64_t> Marked::digests() const {
    std::lock_guard lock(mutex_);
    return taken_;
}

time::Seconds Marked::advance(time::Seconds frontier, time::Seconds goal) {
    // the batch stops at the first mark or job after the frontier
    time::Seconds stop = goal;
    {
        std::lock_guard lock(mutex_);
        const auto mark = marks_.upper_bound(frontier);
        if (mark != marks_.end()) {
            stop = std::min(stop, *mark);
        }
        const auto job = jobs_.upper_bound(frontier);
        if (job != jobs_.end()) {
            stop = std::min(stop, job->first);
        }
    }
    const time::Seconds reached = world_.advance(frontier, stop);
    KD_CHECK(reached <= stop, "run::Marked: a world went past its mark");
    // there: the jobs due, in the order given, then the digest
    std::vector<std::function<void()>> due;
    bool wanted = false;
    {
        std::lock_guard lock(mutex_);
        const auto [first, last] = jobs_.equal_range(reached);
        for (auto it = first; it != last; ++it) {
            due.push_back(std::move(it->second));
        }
        jobs_.erase(first, last);
        wanted = marks_.contains(reached) && !taken_.contains(reached);
    }
    for (const std::function<void()>& job : due) {
        job();
    }
    if (wanted) {
        const std::uint64_t d = digest_();
        std::lock_guard lock(mutex_);
        taken_[reached] = d;
    }
    return reached;
}

}  // namespace kd::run
