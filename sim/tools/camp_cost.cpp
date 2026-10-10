// A19/P1: clocks and file measurements are outputs of the host tool, never inputs
// to the world's laws. Pin this process externally; keeper threads inherit it.
#include "camp_cost.hpp"
#include <sys/resource.h>
#include <time.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <charconv>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <optional>
#include <utility>
#include <vector>
#include "kd/data/craft.hpp"
#include "kd/data/folder.hpp"
#include "kd/demo/crowd_world.hpp"
#include "kd/demo/kept.hpp"
#include "kd/save/files.hpp"
#include "kd/save/keeper.hpp"
namespace kd::tool {
namespace {
using Clock = std::chrono::steady_clock;
std::uint64_t cpu_ns() {
    timespec t{};
    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t) != 0) std::abort();
    return static_cast<std::uint64_t>(t.tv_sec) * 1000000000ULL + static_cast<std::uint64_t>(t.tv_nsec);
}
double process_cpu() {
    rusage usage{};
    getrusage(RUSAGE_SELF, &usage);
    return double(usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) +
           double(usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000000.0;
}
long peak_kib() {
    rusage usage{};
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;
}
std::uint64_t folder_bytes(const std::string& folder) {
    std::error_code error;
    std::uint64_t bytes = 0;
    for (auto it = std::filesystem::recursive_directory_iterator(folder, error);
         !error && it != std::filesystem::recursive_directory_iterator(); it.increment(error))
        if (it->is_regular_file(error)) bytes += it->file_size(error);
    if (error) std::abort();
    return bytes;
}
constexpr std::array<std::string_view, static_cast<std::size_t>(world::Cost::count)> names{
    "people", "camp", "layers", "chooser", "inputs", "thermal", "food", "retention", "observation", "evidence"};
class Probe final : public world::CostProbe {
    struct Frame {
        world::Cost kind;
        std::uint64_t began, child;
    };
    static thread_local std::vector<Frame> stack;

public:
    std::array<std::atomic<std::uint64_t>, names.size()> ns{};
    void enter(world::Cost kind) noexcept override { stack.push_back({kind, cpu_ns(), 0}); }
    void leave(world::Cost kind) noexcept override {
        const auto ended = cpu_ns();
        const auto frame = stack.back();
        KD_CHECK(frame.kind == kind, "balanced host cost scopes");
        stack.pop_back();
        const auto elapsed = ended - frame.began;
        ns[static_cast<std::size_t>(kind)].fetch_add(elapsed - frame.child, std::memory_order_relaxed);
        if (!stack.empty()) stack.back().child += elapsed;
    }
};
thread_local std::vector<Probe::Frame> Probe::stack;
bool number(std::string_view text, std::uint64_t& value) {
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}
}  // namespace
int camp_cost(std::span<const std::string_view> args) {
    std::uint64_t seed = 0, years = 0, wall_limit = 0, calibration_days = 0;
    if (args.size() < 3 || !number(args[0], seed) || !number(args[1], years) || years == 0 || years > 250) return 2;
    bool scalar = false, probing = true, food_accounting = false;
    for (std::size_t n = 3; n < args.size(); ++n) {
        if (args[n] == "--scalar")
            scalar = true;
        else if (args[n] == "--no-probe")
            probing = false;
        else if (args[n] == "--food-accounting")
            food_accounting = true;
        else if (args[n] == "--days" && n + 1 < args.size()) {
            ++n;
            if (!number(args[n], calibration_days) || calibration_days == 0 || calibration_days > years * 60) return 2;
        } else if (args[n] == "--wall-limit" && n + 1 < args.size()) {
            ++n;
            if (!number(args[n], wall_limit) || wall_limit == 0) return 2;
        } else
            return 2;
    }
    const std::string folder(args[2]);
    std::error_code error;
    if (std::filesystem::exists(folder, error) || error) {
        std::fprintf(stderr, "cost report requires a new folder: %s\n", folder.c_str());
        return 2;
    }
    std::filesystem::create_directories(folder, error);
    if (error) {
        std::fprintf(stderr, "cost folder: %s\n", error.message().c_str());
        return 1;
    }
    data::Catalogue catalogue;
    const auto problems = catalogue.load(data::read_catalogue("data"));
    if (!problems.empty()) {
        for (const auto& problem : problems) std::fprintf(stderr, "%s\n", data::problem_text(problem).c_str());
        return 1;
    }
    Probe probe;
    const auto began = Clock::now();
    const auto main_began = cpu_ns();
    const auto process_began = process_cpu();
    demo::CrowdWorld camp(seed, catalogue, 1, true, true);
    auto& w = camp.world();
    w.set_scalar_work(scalar);
    if (probing) w.set_cost_probe(&probe);
    save::DiskFiles files(folder);
    save::Keeper keeper(files, "M4 cost report");
    const auto found = keeper.open();
    if (!found.problem.empty()) {
        std::fprintf(stderr, "%s\n", found.problem.c_str());
        return 1;
    }
    keeper.begin(found, catalogue);
    keeper.about(demo::about_text({"Cost report", seed, 1, false, true, true, {}, false}));
    keeper.keep_kinds([&](const world::Record& record) { return w.keeps(record); });
    std::vector<world::Record> history;
    w.keep_history(&history);
    std::uint64_t storage_main = 0;
    std::map<ecs::Id, std::uint32_t> meals;
    std::map<std::uint32_t, std::uint64_t> finite_intake;
    std::map<std::pair<std::uint32_t, std::uint32_t>, std::pair<std::uint64_t, std::uint64_t>> craft_sources;
    std::map<ecs::Id, std::uint64_t> result_cursor;
    const auto recipe_names = catalogue.names("blueprint");
    const auto item_names = catalogue.names("item");
    const auto item_kind = [&](ecs::Id id) -> std::optional<std::uint32_t> {
        const auto& constant = std::as_const(w);
        if (const auto h = constant.things().find(id)) return constant.things().raw().get<world::Item>(*h).kind;
        if (const auto* item = constant.archived_item(id)) return item->item.kind;
        return {};
    };
    const auto snapshot = [&] {
        const auto before = cpu_ns();
        keeper.snapshot(w);
        keeper.flush();
        storage_main += cpu_ns() - before;
    };
    snapshot();
    auto next_save = began + std::chrono::seconds(30);
    auto wall_previous = began;
    auto main_previous = main_began;
    auto process_previous = process_began;
    std::uint64_t bytes_previous = 0, storage_previous = 0;
    auto visits_previous = w.item_visits();
    bool timed_out = false;
    const auto end = calibration_days ? static_cast<time::Seconds>(calibration_days) * time::kDay
                                      : static_cast<time::Seconds>(years) * time::kYear;
    // One-hour calls include live-index/retention work at the stepper's maximum
    // quantum. Real history, initial/yearly checkpoints and 30-second saves run.
    for (time::Seconds at = time::kHour; at <= end; at += time::kHour) {
        w.run_to(at);
        if (at % (10 * time::kDay) == 0)
            std::fprintf(stderr, "progress seed %llu: day %lld, %.2f s\n", static_cast<unsigned long long>(seed),
                         static_cast<long long>(at / time::kDay),
                         std::chrono::duration<double>(Clock::now() - began).count());
        // Output-only food accounting. Choices are still within their recent
        // window, and physical identities resolve through live or archived facts.
        if (food_accounting) {
            const auto& constant_now = std::as_const(w);
            for (const auto& record : history) {
                if (record.what == 217) {
                    const auto person = constant_now.beings().handle(ecs::Id{record.a});
                    const auto home = constant_now.beings().raw().get<demo::Home>(person).camp;
                    const auto& decisions =
                        constant_now.beings().raw().get<world::CraftHistory>(constant_now.beings().handle(home));
                    const auto* choice = decisions.choices.find(record.b);
                    if (choice && !choice->reasons.empty()) {
                        const auto& selected = choice->reasons.front();
                        if (selected.kind == 2 && selected.need == 0 && !selected.inputs.empty())
                            if (const auto kind = item_kind(selected.inputs.front().id))
                                meals[ecs::Id{record.a}] = *kind;
                    }
                } else if (record.what == 202) {
                    const auto meal = meals.find(ecs::Id{record.a});
                    KD_CHECK(meal != meals.end(), "a finite meal follows its recorded item choice");
                    finite_intake[meal->second] += record.b;
                }
            }
            for (const auto h : constant_now.beings().raw().view<world::CraftHistory>()) {
                const auto camp_id = constant_now.beings().id_of(h);
                const auto& events = constant_now.beings().raw().get<world::CraftHistory>(h).events;
                auto first = std::upper_bound(events.begin(), events.end(), result_cursor[camp_id],
                                              [](auto id, const auto& event) { return id < event.id; });
                for (; first != events.end(); ++first) {
                    const auto& event = *first;
                    result_cursor[camp_id] = event.id;
                    if ((event.kind != 0 && event.kind != 1 && event.kind != 5) || event.inputs.empty()) continue;
                    const auto source = item_kind(event.inputs.front().id);
                    KD_CHECK(source.has_value(), "a recorded input has stable physical facts");
                    auto& counts = craft_sources[{event.recipe, *source}];
                    ++counts.first;
                    if (const auto result = item_kind(event.result); result && *result == *source) ++counts.second;
                }
            }
        }
        const auto store_began = cpu_ns();
        keeper.history(history);
        history.clear();
        storage_main += cpu_ns() - store_began;
        if (Clock::now() >= next_save) {
            snapshot();
            next_save = Clock::now() + std::chrono::seconds(30);
        }
        if (keeper.failed()) {
            std::fprintf(stderr, "cost keeper write failed\n");
            return 1;
        }
        timed_out = wall_limit && std::chrono::duration<double>(Clock::now() - began).count() >= double(wall_limit);
        if (at != end && at % time::kYear != 0 && !timed_out) continue;
        if (keeper.last_snapshot() != at) snapshot();
        if (keeper.failed()) {
            std::fprintf(stderr, "cost keeper write failed\n");
            return 1;
        }
        const auto bytes = folder_bytes(folder);
        const auto digests = w.digests();
        const auto digest = digests.whole;
        const auto now = Clock::now();
        const auto main_now = cpu_ns();
        const auto process_now = process_cpu();
        const auto& constant = std::as_const(w);
        std::size_t people = 0, choices = 0, results = 0, eligible_spent = 0;
        for (const auto h : constant.beings().raw().view<world::Person>()) {
            (void)h;
            ++people;
        }
        for (const auto h : constant.beings().raw().view<world::CraftHistory>()) {
            const auto& craft = constant.beings().raw().get<world::CraftHistory>(h);
            choices += craft.choices.size();
            results += craft.events.size();
        }
        for (const auto h : constant.things().raw().view<world::Item>()) {
            const auto& item = constant.things().raw().get<world::Item>(h);
            if (item.mass == 0 && item.state == 4 && !constant.things().raw().all_of<world::Fire>(h)) ++eligible_spent;
        }
        for (const auto h : constant.beings().raw().view<world::CraftHistory>()) {
            std::vector<std::uint64_t> counts(catalogue.kind<data::Blueprint>().size());
            for (const auto& event : constant.beings().raw().get<world::CraftHistory>(h).events) ++counts[event.recipe];
            for (std::size_t recipe = 0; recipe < counts.size(); ++recipe)
                if (counts[recipe])
                    std::fprintf(stderr, "results recipe %zu: %llu\n", recipe,
                                 static_cast<unsigned long long>(counts[recipe]));
        }
        const auto visits = w.item_visits();
        const auto elapsed = std::chrono::duration<double>(now - began).count();
        const auto period = std::chrono::duration<double>(now - wall_previous).count();
        const auto main = double(main_now - main_previous) / 1e9;
        const auto storage = double(storage_main - storage_previous) / 1e9;
        std::printf(
            "{\"seed\":%llu,\"game_years\":%.9f,\"complete\":%s,\"wall_seconds\":%.6f,\"period_seconds\":%.6f,"
            "\"years_per_minute\":%.6f,\"main_cpu_seconds\":%.6f,\"process_cpu_seconds\":%.6f,"
            "\"peak_rss_kib\":%ld,\"population\":%zu,\"live_items\":%zu,\"archived_items\":%zu,"
            "\"eligible_spent\":%zu,\"choices\":%zu,\"results\":%zu,\"folder_bytes\":%llu,\"bytes_added\":%lld,"
            "\"snapshot_bytes\":%llu,\"reachable_visits\":%llu,\"thermal_visits\":%llu,\"digest\":\"%s\",\"cpu\":{",
            static_cast<unsigned long long>(seed), double(at) / double(time::kYear), at == end ? "true" : "false",
            elapsed, period, double(at) / double(time::kYear) * 60.0 / elapsed, main, process_now - process_previous,
            peak_kib(), people, constant.things().size(), constant.item_archive().size(), eligible_spent, choices,
            results, static_cast<unsigned long long>(bytes),
            static_cast<long long>(bytes) - static_cast<long long>(bytes_previous),
            static_cast<unsigned long long>(keeper.last_snapshot_bytes()),
            static_cast<unsigned long long>(visits.reachable - visits_previous.reachable),
            static_cast<unsigned long long>(visits.thermal - visits_previous.thermal), num::to_hex(digest).c_str());
        double scoped = 0.0;
        for (std::size_t n = 0; n < names.size(); ++n) {
            const auto seconds = double(probe.ns[n].exchange(0, std::memory_order_relaxed)) / 1e9;
            scoped += seconds;
            std::printf("\"%.*s\":%.6f,", int(names[n].size()), names[n].data(), seconds);
        }
        std::printf("\"storage\":%.6f,\"queue_reporting\":%.6f,\"io_thread\":%.6f},", storage,
                    std::max(0.0, main - storage - scoped), std::max(0.0, process_now - process_previous - main));
        std::printf(
            "\"physical_parts\":{\"clock\":\"%s\",\"queue\":\"%s\",\"things\":\"%s\",\"systems\":\"%s\",\"history\":\"%"
            "s\"},",
            num::to_hex(digests.clock).c_str(), num::to_hex(digests.queue).c_str(), num::to_hex(digests.things).c_str(),
            num::to_hex(digests.systems).c_str(), num::to_hex(digests.history).c_str());
        std::printf("\"finite_intake_mg\":{");
        bool separator = false;
        for (const auto& [kind, mass] : finite_intake) {
            std::printf("%s\"%s\":%llu", separator ? "," : "", item_names[kind].c_str(),
                        static_cast<unsigned long long>(mass));
            separator = true;
        }
        std::printf("},\"craft_sources\":[");
        separator = false;
        for (const auto& [key, counts] : craft_sources) {
            std::printf("%s{\"recipe\":\"%s\",\"source\":\"%s\",\"recorded_results\":%llu,\"same_kind_results\":%llu}",
                        separator ? "," : "", recipe_names[key.first].c_str(), item_names[key.second].c_str(),
                        static_cast<unsigned long long>(counts.first), static_cast<unsigned long long>(counts.second));
            separator = true;
        }
        std::printf("]}\n");
        std::fflush(stdout);
        std::fprintf(stderr, "seed %llu: %.3f years, %.2f s, %.2f years/min, peak %.1f MiB\n",
                     static_cast<unsigned long long>(seed), double(at) / double(time::kYear), elapsed,
                     double(at) / double(time::kYear) * 60.0 / elapsed, double(peak_kib()) / 1024.0);
        wall_previous = now;
        main_previous = main_now;
        process_previous = process_now;
        bytes_previous = bytes;
        storage_previous = storage_main;
        visits_previous = visits;
        if (timed_out) break;
    }
    w.set_cost_probe(nullptr);
    w.keep_history(nullptr);
    return timed_out ? 3 : 0;
}
}  // namespace kd::tool
