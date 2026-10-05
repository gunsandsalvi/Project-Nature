#include <atomic>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

#include "doctest.h"
#include "helpers.hpp"
#include "kd/num/fenv.hpp"
#include "kd/proof/proof.hpp"
#include "kd/run/workers.hpp"

// checks: PLT-01
TEST_CASE("every piece runs exactly once, on any number of threads") {
    for (int threads = 1; threads <= 4; ++threads) {
        kd::run::Workers workers(threads);
        std::vector<std::atomic<int>> runs(1000);
        workers.for_each(runs.size(), [&](std::size_t i) { runs[i].fetch_add(1); });
        for (const auto& r : runs) {
            CHECK(r.load() == 1);
        }
    }
}

// checks: PLT-01
TEST_CASE("workers have the stack they were given") {
    kd::run::Workers workers(2);
    for (std::size_t size : workers.stack_sizes()) {
        CHECK(size >= (std::size_t{8} << 20));
    }
}

// checks: RES-05
TEST_CASE("workers set the default floating-point environment, whatever their creator had") {
    std::vector<std::uint64_t> inherited;
    bool default_in_work = false;
    std::thread creator([&] {
        kd::test::set_flush_to_zero();
        kd::run::Workers workers(1);
        inherited = workers.inherited_fenv();
        workers.for_each(1, [&](std::size_t) { default_in_work = kd::num::fenv_is_default(kd::num::fenv_read()); });
    });
    creator.join();
    REQUIRE(inherited.size() == 1);
    CHECK_FALSE(kd::num::fenv_is_default(inherited[0]));
    CHECK(default_in_work);
}

// checks: RES-05
TEST_CASE("the smoke suite gives one digest on one to four threads") {
    std::vector<std::string> digests;
    for (int threads = 1; threads <= 4; ++threads) {
        kd::run::Workers workers(threads);
        digests.push_back(kd::proof::run("smoke", workers));
    }
    for (const auto& d : digests) {
        CHECK(d.size() == 16);
        CHECK(d == digests.front());
    }
}

// checks: RES-05
TEST_CASE("an unknown suite gives no digest") {
    kd::run::Workers workers(1);
    CHECK(kd::proof::run("no such suite", workers).empty());
}
