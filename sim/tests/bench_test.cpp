#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "doctest.h"
#include "helpers.hpp"
#include "kd/bench/code.hpp"
#include "kd/bench/scenarios.hpp"
#include "kd/demo/clockwork.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/run/marked.hpp"

namespace {

// A full set of measures, as a phone might give them.
kd::bench::Values measured() {
    kd::bench::Values v{{"build", 20502}, {"cores", 9},   {"big_mhz", 3400}, {"refresh_hz", 120}, {"android", 16},
                        {"battery", 87},  {"plugged", 1}, {"thermal", 1},    {"seconds", 961}};
    for (const kd::bench::Scenario& s : kd::bench::scenarios()) {
        const std::string n(s.name);
        v[n + ".on_time"] = 0.987;
        v[n + ".slowest"] = 41;
        v[n + ".stalls"] = 12;
        v[n + ".speed"] = 172'800;
        v[n + ".digest"] = 1;
        v[n + ".digest_bits"] = 0xABCDE;
        v[n + ".heat"] = 0.62;
        v[n + ".share"] = 100;
        v[n + ".current"] = 612;
        v[n + ".memory"] = 840;
        v[n + ".cpu"] = 71;
        v[n + ".draw_ms"] = 1.07;
        v[n + ".clock"] = 2850;
    }
    v["saves.save_ms"] = 4;
    v["saves.export_ms"] = 23;
    v["saves.open_ms"] = 412;
    return v;
}

// Every letter of a code, its dashes left out.
std::string letters_of(const std::string& code) {
    std::string out;
    for (const char c : code) {
        if (c != '-') {
            out += c;
        }
    }
    return out;
}

}  // namespace

// checks: PLT-04
TEST_CASE("a code holds each measure to its field's step, and leaves out what was not measured") {
    const kd::bench::Values in = measured();
    const std::string code = kd::bench::encode(in);
    const kd::bench::Read read = kd::bench::decode(code);
    INFO(read.why);
    REQUIRE(read.why.empty());
    CHECK(read.values.at("build") == 20502);
    CHECK(read.values.at("big_mhz") == 3400);
    CHECK(read.values.at("top.on_time") == doctest::Approx(0.987));
    CHECK(read.values.at("top.slowest") == 41);
    // a speed to within 4.4%, a sixteenth of a doubling
    CHECK(read.values.at("top.speed") == doctest::Approx(172'800).epsilon(0.045));
    CHECK(read.values.at("top.digest_bits") == 0xABCDE);
    CHECK(read.values.at("top.heat") == doctest::Approx(0.62));
    CHECK(read.values.at("top.draw_ms") == doctest::Approx(1.07));
    CHECK(read.values.at("top.clock") == 2850);
    CHECK(read.values.at("saves.open_ms") == 412);
    // a measure missing is left out; one beyond its field's range is held at its most
    kd::bench::Values some = in;
    some.erase("top.memory");
    some["real.slowest"] = 99'999;
    const kd::bench::Read partial = kd::bench::decode(kd::bench::encode(some));
    CHECK_FALSE(partial.values.contains("top.memory"));
    CHECK(partial.values.at("real.slowest") == 2046);
    CHECK(partial.values.at("real.memory") == 840);
}

// checks: PLT-04
TEST_CASE(
    "a code made in the cloud decodes after its case, line breaks and dashes change, and a wrong letter is caught") {
    const std::string code = kd::bench::encode(measured());
    CHECK(code.find_first_of("*_~") == std::string::npos);
    const kd::bench::Values want = kd::bench::decode(code).values;
    REQUIRE_FALSE(want.empty());
    // lower case, broken into lines, its dashes swapped for others a chat app might put, or gone
    std::string lower;
    for (const char c : code) {
        lower += c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : c;
    }
    CHECK(kd::bench::decode(lower).values == want);
    std::string broken;
    for (std::size_t i = 0; i < code.size(); ++i) {
        broken += code[i];
        if (i % 17 == 16) {
            broken += i % 2 == 0 ? "\n" : "\r\n  ";
        }
    }
    CHECK(kd::bench::decode(broken).values == want);
    for (const char* dash : {"", " ", "–", "—", "−", "‑", " "}) {
        std::string swapped;
        for (const char c : code) {
            swapped += c == '-' ? std::string(dash) : std::string(1, c);
        }
        CAPTURE(dash);
        CHECK(kd::bench::decode(swapped).values == want);
    }
    // Crockford's look-alikes: o for 0, i and l for 1
    std::string alike = letters_of(code);
    for (char& c : alike) {
        c = c == '0' ? 'o' : c == '1' ? 'l' : c;
    }
    CHECK(kd::bench::decode(alike).values == want);
    // every letter changed to every other is caught, and so is a letter left out or one too many
    const std::string plain = letters_of(code);
    const std::string alphabet = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
    std::size_t caught = 0;
    std::size_t tried = 0;
    for (std::size_t i = 0; i < plain.size(); ++i) {
        for (const char c : alphabet) {
            if (c == plain[i]) {
                continue;
            }
            std::string wrong = plain;
            wrong[i] = c;
            ++tried;
            caught += kd::bench::decode(wrong).why.empty() ? 0 : 1;
        }
    }
    CHECK(caught == tried);
    CHECK_FALSE(kd::bench::decode(plain.substr(1)).why.empty());
    CHECK_FALSE(kd::bench::decode(plain + "0").why.empty());
    // and a letter no code has is named
    CHECK(kd::bench::decode(plain.substr(0, 10) + "!" + plain.substr(10)).why.find('!') != std::string::npos);
}

// checks: RES-05
TEST_CASE("a world with marks takes each digest as it passes, the same however its batches fall") {
    const kd::data::Catalogue& cat = kd::test::fixture();
    const auto run_marked = [&](const std::vector<kd::time::Seconds>& goals) {
        kd::demo::CrowdWorld crowd(5, cat, 2);
        struct Steps : kd::run::Steppable {
            kd::world::World& w;
            explicit Steps(kd::world::World& world) : w(world) {}
            kd::time::Seconds advance(kd::time::Seconds frontier, kd::time::Seconds goal) override {
                return w.advance(frontier, std::min(goal, frontier + 777));
            }
        } steps(crowd.world());
        kd::run::Marked marked(steps, [&] { return crowd.world().digests().whole; });
        marked.mark(30'000);
        marked.mark(2 * kd::time::kDay);
        marked.at(50'000, [&] {
            (void)crowd.world().command(50'000, static_cast<std::uint32_t>(kd::demo::Commanded::call_home),
                                        crowd.camp_ids().front().value, 0);
        });
        kd::time::Seconds at = 0;
        for (const kd::time::Seconds goal : goals) {
            while (at < goal) {
                at = marked.advance(at, goal);
            }
        }
        return marked.digests();
    };
    const auto want = run_marked({3 * kd::time::kDay});
    REQUIRE(want.size() == 2);
    CHECK(run_marked({29'999, 30'001, 49'999, 90'000, 3 * kd::time::kDay}) == want);
    // and as a world run straight there, its command given at its second
    kd::demo::CrowdWorld crowd(5, cat, 2);
    crowd.world().run_to(30'000);
    CHECK(crowd.world().digests().whole == want.at(30'000));
    crowd.world().run_to(50'000);
    (void)crowd.world().command(50'000, static_cast<std::uint32_t>(kd::demo::Commanded::call_home),
                                crowd.camp_ids().front().value, 0);
    crowd.world().run_to(2 * kd::time::kDay);
    CHECK(crowd.world().digests().whole == want.at(2 * kd::time::kDay));
}
