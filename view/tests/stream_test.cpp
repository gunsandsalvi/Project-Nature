#include "stream.hpp"
#include "doctest.h"

using namespace kd::view;
namespace {
StreamIdentity identity(std::uint64_t epoch = 1) {
    return {"world", "data", "look", "renderer", 1, epoch};
}
StreamRequest request_of(std::string key, std::string parent = "", std::string category = "sprites") {
    StreamRequest r;
    r.identity = identity();
    r.key = std::move(key);
    r.parent = std::move(parent);
    r.category = std::move(category);
    r.input_bytes = 10;
    r.prepared_bytes = 30;
    r.resident_bytes = 60;
    r.revision = 1;
    r.input.encoded = {1, 2, 3};
    return r;
}
void opened(StreamState& s, StreamLimits limits = {}) {
    s.begin(identity(), std::move(limits));
    CHECK(s.manifest({1, 1, 5.0, {{"surface", {{"tile", 1}}}}}));
}
bool complete(StreamState& s, StreamToken token) {
    if (!s.ready(token, 30)) return false;
    for (const auto* channel : {"colour", "normal", "material"}) {
        if (!s.stage(token, channel, 20) || !s.uploaded(token, channel, 20)) return false;
    }
    return s.publish(token);
}
}  // namespace

// checks: PLT-04 WLD-13 (T2.9a.2): cancelling a worker never pretends its memory was freed.
TEST_CASE("cancelled preparing jobs retain owned byte reservations through rapid world changes") {
    StreamState s;
    StreamLimits limits;
    limits.input = 10;
    limits.prepared = 30;
    opened(s, limits);
    const auto token = s.request(request_of("one"));
    CHECK(token != 0);
    auto work = s.take_jobs(1);
    CHECK(work.size() == 1);
    CHECK(s.cancel(token));
    CHECK(s.ledger().input == 10);
    CHECK(s.ledger().prepared == 30);
    s.begin(identity(2), limits);
    CHECK(s.ledger().input == 10);
    CHECK(s.ledger().prepared == 30);
    auto next = request_of("two");
    next.identity = identity(2);
    next.revision = 0;
    CHECK(s.request(next) == 0);
    CHECK_FALSE(s.ready(token, 30));
    CHECK(s.disposed(token, "input"));
    CHECK(s.disposed(token, "prepared"));
    CHECK(s.ledger().input == 0);
    CHECK(s.ledger().prepared == 0);
    CHECK(s.request(next) != 0);
}

// checks: PRE-03 PRE-22 PLT-04 (T2.9a.2): staged uploads are never partly visible and parent remains pinned.
TEST_CASE("aligned channel bundles publish atomically with pinned coarse parent and overlap accounting") {
    StreamState s;
    opened(s);
    const auto parent = s.request(request_of("parent"));
    s.take_jobs(1);
    CHECK(complete(s, parent));
    const auto child = s.request(request_of("child", "parent"));
    s.take_jobs(1);
    CHECK_FALSE(s.evict("parent"));
    CHECK(s.ready(child, 30));
    CHECK(s.stage(child, "colour", 20));
    CHECK(s.uploaded(child, "colour", 20));
    CHECK_FALSE(s.publish(child));
    CHECK(s.ledger().resident == 80);
    for (const auto* channel : {"normal", "material"}) {
        CHECK(s.stage(child, channel, 20));
        CHECK(s.uploaded(child, channel, 20));
    }
    CHECK(s.publish(child));
    CHECK(s.ledger().resident == 120);
    CHECK(s.evict("parent"));
    CHECK(s.ledger().resident == 120);
    CHECK(s.disposed(parent, "gpu"));
    CHECK(s.ledger().resident == 60);
}

// checks: WLD-13 PLT-04 (T2.9a.2): complete manifests recover skipped revisions without rejecting unrelated art.
TEST_CASE("complete revision manifests reject stale dependencies and retain unrelated prepared work") {
    StreamState s;
    opened(s);
    auto r = request_of("tile");
    r.dependencies = {{"surface", "tile", 1}};
    const auto token = s.request(r);
    s.take_jobs(1);
    CHECK(s.manifest({1, 5, 25.0, {{"surface", {{"tile", 1}, {"other", 9}}}}}));
    CHECK(s.ready(token, 30));
    CHECK(s.manifest({1, 9, 45.0, {{"surface", {{"tile", 2}}}}}));
    CHECK_FALSE(s.stage(token, "colour", 20));
    CHECK(s.ledger().prepared == 30);
    CHECK(s.disposed(token, "prepared"));
    CHECK_FALSE(s.manifest({1, 8, 40.0, {}}));
}

// checks: PLT-04 (T2.9a.2): category allocations and future old/new overlap cannot borrow other categories' budgets.
TEST_CASE("stream budgets include category reservations staging and separately allocated targets") {
    StreamState s;
    StreamLimits limits;
    limits.resident = 200;
    limits.staging = 19;
    limits.targets = 32;
    limits.resident_by_category = {{"sprites", 60}, {"ground", 60}};
    opened(s, limits);
    const auto one = s.request(request_of("one"));
    CHECK(one != 0);
    CHECK(s.request(request_of("two")) == 0);
    CHECK(s.request(request_of("ground", "", "ground")) != 0);
    s.take_jobs(1);
    CHECK(s.ready(one, 30));
    CHECK_FALSE(s.stage(one, "colour", 20));
    CHECK(s.reserve_target(32));
    CHECK_FALSE(s.reserve_target(1));
    CHECK(s.ledger().targets == 32);
    CHECK(s.release_target(32));
    CHECK(s.ledger().targets == 0);
    CHECK_FALSE(s.release_target(1));
}

// checks: PLT-04 WLD-13 (T2.9a.2): in-flight uploads retain their future allocation claim after cancellation.
TEST_CASE("cancelling an upload retains headroom and accounts a late allocated GPU channel until disposal") {
    StreamState s;
    StreamLimits limits;
    limits.resident = 60;
    opened(s, limits);
    const auto token = s.request(request_of("one"));
    s.take_jobs(1);
    CHECK(s.ready(token, 30));
    CHECK(s.stage(token, "colour", 20));
    CHECK(s.cancel(token));
    CHECK(s.ledger().reserved_resident == 20);
    CHECK(s.request(request_of("two")) == 0);
    CHECK_FALSE(s.uploaded(token, "colour", 20));
    CHECK(s.ledger().resident == 20);
    CHECK(s.ledger().staging == 0);
    CHECK(s.disposed(token, "gpu"));
    CHECK(s.ledger().resident == 0);
    CHECK(s.disposed(token, "input"));
    CHECK(s.disposed(token, "prepared"));
    CHECK(s.request(request_of("two")) != 0);
}

// checks: PLT-04 WLD-13 (T2.9a.2): retained identity metadata still owns bytes after encoded inputs are dropped.
TEST_CASE("published job metadata remains charged until final resource disposal") {
    StreamState s;
    opened(s);
    auto request = request_of("resident");
    request.metadata_bytes = 4;
    const auto token = s.request(std::move(request));
    s.take_jobs(1);
    CHECK(complete(s, token));
    CHECK(s.disposed(token, "input"));
    CHECK(s.ledger().input == 4);
    CHECK(s.disposed(token, "input"));
    CHECK(s.ledger().input == 4);
    CHECK(s.disposed(token, "prepared"));
    CHECK(s.ledger().input == 4);
    CHECK(s.cancel(token));
    CHECK(s.ledger().input == 4);
    CHECK(s.disposed(token, "gpu"));
    CHECK(s.ledger().input == 0);
    CHECK(s.job(token) == nullptr);
}
