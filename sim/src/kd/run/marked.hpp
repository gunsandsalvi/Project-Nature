// Marks in a world's game time (A18.1): game seconds at which a running world's digest is taken as it passes, and
// your commands given at exactly their seconds, for the benchmark, whose digests must be the cloud's (RES-05). The
// world's batches stop at each mark; since a cut between batches never changes the result, the marks change nothing
// but where the batches end.
#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <set>

#include "kd/run/runner.hpp"

namespace kd::run {

/// Implements RES-05 and PLT-04, see A18.1: a world with marks, as its runner steps it.
class Marked final : public Steppable {
public:
    /// A world, and how to read its digest between batches.
    Marked(Steppable& world, std::function<std::uint64_t()> digest);

    /// Takes the world's digest when it reaches this game second; from any thread, before the world passes it.
    void mark(time::Seconds at);
    /// Does this on the world's thread when it reaches this game second, before its digest there is taken, if both.
    void at(time::Seconds second, std::function<void()> job);
    /// The digest taken at a mark, once the world has passed it.
    [[nodiscard]] std::optional<std::uint64_t> digest_at(time::Seconds mark) const;
    /// Every digest taken so far, by game second.
    [[nodiscard]] std::map<time::Seconds, std::uint64_t> digests() const;

    /// A batch of the world's, stopped at the next mark, where the mark's job and digest are taken.
    time::Seconds advance(time::Seconds frontier, time::Seconds goal) override;

private:
    Steppable& world_;
    std::function<std::uint64_t()> digest_;
    mutable std::mutex mutex_;
    std::set<time::Seconds> marks_;
    std::multimap<time::Seconds, std::function<void()>> jobs_;
    std::map<time::Seconds, std::uint64_t> taken_;
};

}  // namespace kd::run
