// The crowd's scenes (A17, RES-21): what a run of the demonstration's crowd is measured by and held to, and one run
// kept in a folder of its own, a checkpoint at each game day's end, so a run cut off by a restart resumes as if it had
// never stopped (PLT-05). The never rules are checked on what each day's end samples, and the measures are taken from
// the samples and from the history the folder keeps, so a resumed run counts exactly as an unbroken one.
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "kd/data/catalogue.hpp"
#include "kd/save/files.hpp"
#include "kd/scene/scene.hpp"

namespace kd::demo {

/// The crowd as a kind of world for scenes: its measures and its rules never to break.
[[nodiscard]] const scene::WorldKind& crowd_kind();

/// What the run's caller does at each game day's end, as the tool reads the memory it holds (RES-12).
using EachDay = std::function<void(std::int64_t day)>;

/// Implements RES-21, RES-12 and PLT-05, see A17: one run of a crowd's scene in its folder, by its index: made new,
/// a test world with its switches, or opened from its last checkpoint and caught up; run to the scene's end a game
/// day at a time, a checkpoint at each day's end, saved under the build's name, the app's version for a world the
/// phone opens as its own; then measured, held to its never rules and expected ranges, and opened again to check that
/// the save opens as the world was.
scene::RunResult run_crowd(const scene::Scene& s, std::int64_t index, save::Files& folder,
                           const data::Catalogue& catalogue, const std::string& build, const EachDay& each_day = {});

}  // namespace kd::demo
