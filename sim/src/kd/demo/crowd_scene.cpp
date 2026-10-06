#include "kd/demo/crowd_scene.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <optional>
#include <string>

#include "kd/demo/crowd_world.hpp"
#include "kd/demo/kept.hpp"
#include "kd/save/keeper.hpp"
#include "kd/time/calendar.hpp"

namespace kd::demo {

namespace {

constexpr std::array<scene::Named, 4> kMeasures{{
    {"greetings", "the greetings in the run"},
    {"greetings_per_camp_day", "the greetings a camp a game day, rounded down"},
    {"farthest_from_home", "the farthest any marker was from its camp at a day's end, in metres"},
    {"awake_at_midnight", "the most markers awake at any midnight"},
}};
constexpr std::array<scene::Named, 2> kNevers{{
    {"farther_from_home", "no marker farther from its camp at a day's end than the limit, in metres"},
    {"awake_at_midnight", "no marker awake at midnight for as many nights running as the limit"},
}};
const scene::WorldKind kCrowdKind{"crowd", kMeasures, kNevers};

// The run's files beside its world's: each day's sample, and what it ended with.
const std::string kDays = "days.txt";

// A game day's end as the run samples it: the farthest marker from its camp, and the markers awake.
struct Sample {
    std::int64_t day = 0;
    std::int64_t farthest = 0;  // centimetres
    std::uint64_t farthest_id = 0;
    std::vector<std::uint64_t> awake;
};

save::Bytes bytes_of(const std::string& text) {
    save::Bytes out;
    for (const char c : text) {
        out.push_back(static_cast<std::byte>(c));
    }
    return out;
}

// days.txt: a line a day, "day farthest farthest_id awake...", every number whole
std::string samples_text(const std::vector<Sample>& samples) {
    std::string out;
    for (const Sample& s : samples) {
        out += std::to_string(s.day) + " " + std::to_string(s.farthest) + " " + std::to_string(s.farthest_id);
        for (const std::uint64_t id : s.awake) {
            out += " " + std::to_string(id);
        }
        out += "\n";
    }
    return out;
}

std::vector<Sample> read_samples(const save::Bytes& bytes) {
    std::vector<Sample> out;
    std::string line;
    const auto finish_line = [&] {
        std::vector<std::uint64_t> numbers;
        std::size_t at = 0;
        while (at < line.size()) {
            const std::size_t end = std::min(line.find(' ', at), line.size());
            numbers.push_back(std::strtoull(line.substr(at, end - at).c_str(), nullptr, 10));
            at = end + 1;
        }
        if (numbers.size() >= 3) {
            Sample s;
            s.day = static_cast<std::int64_t>(numbers[0]);
            s.farthest = static_cast<std::int64_t>(numbers[1]);
            s.farthest_id = numbers[2];
            s.awake.assign(numbers.begin() + 3, numbers.end());
            out.push_back(std::move(s));
        }
        line.clear();
    };
    for (const std::byte b : bytes) {
        if (b == std::byte{'\n'}) {
            finish_line();
        } else {
            line.push_back(static_cast<char>(b));
        }
    }
    return out;
}

Sample sample(const world::World& w, std::int64_t day) {
    Sample out;
    out.day = day;
    const time::Seconds t = w.frontier();
    const auto& raw = w.beings().raw();
    w.beings().each([&](ecs::Id id, world::Beings::Handle h) {
        if (id.family() != ecs::Family::marker) {
            return;
        }
        const world::Activity& a = raw.get<world::Activity>(h);
        const std::int64_t far = w.torus().distance(a.at(w.torus(), t), raw.get<Home>(h).at);
        if (far > out.farthest) {
            out.farthest = far;
            out.farthest_id = id.value;
        }
        if (a.what != static_cast<std::uint8_t>(Doing::sleep)) {
            out.awake.push_back(id.value);
        }
    });
    return out;
}

// The greetings the run's history holds, year by year as its folder keeps it.
std::int64_t greetings_in(save::Files& folder) {
    std::int64_t n = 0;
    for (const std::string& name : folder.list("history")) {
        for (const save::Year::Held& h :
             save::read_year(folder.read("history/" + name).value_or(save::Bytes{})).records) {
            n += ecs::Id{h.record.key.owner}.family() == ecs::Family::marker &&
                         h.record.what == static_cast<std::uint32_t>(Happened::greeting)
                     ? 1
                     : 0;
        }
    }
    return n;
}

// The memory a planted leak keeps, never given back (RES-12).
std::vector<std::vector<std::byte>>& leaked() {
    static std::vector<std::vector<std::byte>> kept;
    return kept;
}

}  // namespace

const scene::WorldKind& crowd_kind() {
    return kCrowdKind;
}

scene::RunResult run_crowd(const scene::Scene& s, std::int64_t index, save::Files& folder,
                           const data::Catalogue& catalogue, const std::string& build, const EachDay& each_day) {
    scene::RunResult out;
    out.index = index;
    out.seed = s.seed + index;
    out.switches = s.switches_of(index);
    out.days = (s.until + time::kDay - 1) / time::kDay;
    // a test world, its switches in its world.toml, which the world takes as it is made (RES-10, PLT-05)
    if (!folder.read("world.toml")) {
        About about;
        about.name = s.name + " " + std::to_string(index + 1);
        about.seed = static_cast<std::uint64_t>(out.seed);
        about.camps = s.camps;
        about.test = true;
        for (const world::Switch sw : out.switches) {
            about.switches.emplace_back(world::kSwitchNames[static_cast<std::size_t>(sw)]);
        }
        folder.write_whole("world.toml", bytes_of(about_text(about)));
    }
    std::vector<Sample> samples;
    {
        save::Keeper keeper(folder, build);
        Kept kept = keep_crowd(keeper, catalogue, static_cast<std::uint64_t>(out.seed), s.camps);
        if (!kept.crowd) {
            out.oddities.push_back("its world did not open: " + kept.problem);
            return out;
        }
        world::World& w = kept.crowd->world();
        std::vector<world::Record> records;
        w.keep_history(&records);
        // the days sampled before, up to the checkpoint it resumes from
        const std::int64_t from_day = w.frontier() / time::kDay;
        samples = read_samples(keeper.read(kDays).value_or(save::Bytes{}));
        std::erase_if(samples, [&](const Sample& x) { return x.day > from_day; });
        for (std::int64_t day = from_day + 1; day <= out.days; ++day) {
            w.run_to(std::min(day * time::kDay, s.until));
            keeper.history(records);
            records.clear();
            samples.push_back(sample(w, day));
            // the planted faults the harness itself must catch (RES-12)
            if (w.switched(world::Switch::plant_crash) && day == out.days / 2 + 1) {
                std::abort();
            }
            if (w.switched(world::Switch::plant_leak)) {
                leaked().emplace_back(std::size_t{8} << 20U, std::byte{1});
            }
            if (each_day) {
                each_day(day);
            }
            keeper.write(kDays, bytes_of(samples_text(samples)));
            keeper.snapshot(w);
        }
        keeper.flush();
        out.digest = w.digests().whole;
        if (keeper.mismatches() > 0) {
            out.oddities.push_back("its history was made again differently after it resumed");
        }
    }
    if (out.switches.end() != std::find(out.switches.begin(), out.switches.end(), world::Switch::plant_bad_save)) {
        // the last snapshot damaged, as a planted fault, for the reopening to catch
        const std::vector<std::string> names = folder.list("snapshots");
        if (!names.empty()) {
            save::Bytes raw = folder.read("snapshots/" + names.back()).value_or(save::Bytes{});
            if (!raw.empty()) {
                raw[raw.size() / 2] ^= std::byte{0x40};
                folder.write_whole("snapshots/" + names.back(), raw);
            }
        }
    }
    {
        // the save opens again as the world was
        save::Keeper again(folder, build);
        const Kept reopened = keep_crowd(again, catalogue, static_cast<std::uint64_t>(out.seed), s.camps);
        if (!reopened.crowd || reopened.crowd->world().digests().whole != out.digest) {
            out.oddities.push_back("its last save did not open again as the world was");
        }
    }

    // the measures, from the history kept and the days sampled
    const std::int64_t greetings = greetings_in(folder);
    std::int64_t farthest = 0;
    std::int64_t awake = 0;
    for (const Sample& x : samples) {
        farthest = std::max(farthest, x.farthest / 100);
        awake = std::max(awake, static_cast<std::int64_t>(x.awake.size()));
    }
    out.measures = {{"greetings", greetings},
                    {"greetings_per_camp_day", greetings / std::max<std::int64_t>(1, s.camps * out.days)},
                    {"farthest_from_home", farthest},
                    {"awake_at_midnight", awake}};

    // the never rules, each named at the first day it broke
    for (const scene::Never& n : s.nevers) {
        if (n.rule == "farther_from_home") {
            for (const Sample& x : samples) {
                if (x.farthest / 100 > n.limit) {
                    out.oddities.push_back("day " + std::to_string(x.day) + ": a marker " +
                                           std::to_string(x.farthest / 100) + " m from its camp, farther than " +
                                           std::to_string(n.limit));
                    break;
                }
            }
        } else if (n.rule == "awake_at_midnight") {
            for (std::size_t i = 0; i < samples.size(); ++i) {
                const std::optional<std::uint64_t> sleepless = [&]() -> std::optional<std::uint64_t> {
                    for (const std::uint64_t id : samples[i].awake) {
                        std::int64_t nights = 0;
                        for (std::size_t j = i + 1; j-- > 0 && nights < n.limit;) {
                            if (std::find(samples[j].awake.begin(), samples[j].awake.end(), id) ==
                                samples[j].awake.end()) {
                                break;
                            }
                            ++nights;
                        }
                        if (nights >= n.limit) {
                            return id;
                        }
                    }
                    return std::nullopt;
                }();
                if (sleepless) {
                    out.oddities.push_back("day " + std::to_string(samples[i].day) + ": a marker awake at midnight " +
                                           std::to_string(n.limit) + " nights running");
                    break;
                }
            }
        }
    }
    // and the expected ranges
    for (const scene::Expect& e : s.expects) {
        const std::optional<std::int64_t> v = out.measure(e.measure);
        if (v && (*v < e.from || *v > e.to)) {
            out.oddities.push_back(e.measure + " " + std::to_string(*v) + ", outside its expected " +
                                   std::to_string(e.from) + " to " + std::to_string(e.to));
        }
    }
    return out;
}

}  // namespace kd::demo
