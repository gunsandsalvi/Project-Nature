#!/usr/bin/env python3
"""Prepare observation-only instrumentation of the final M3 source, never the current app.

Build: . tools/env.sh; python3 tools/m4_m3_diagnose.py; cmake --build build/m4-m3-diagnostic -j2 --target m4_m3_diagnose
Run one already judged seed with its original digest: m4_m3_diagnose spread|fire <seed> <digest> <archived data>
The observer must reproduce that digest. Its times include instrumentation, and are not a performance gate.
"""

from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "build/m4-m3-diagnostic-src"
BUILD = ROOT / "build/m4-m3-diagnostic"


def patch(relative, old, new):
    path = SOURCE / "sim/src/kd" / relative
    text = path.read_text()
    if text.count(old) != 1:
        raise RuntimeError(f"diagnostic anchor is ambiguous or missing: {relative}: {old}")
    path.write_text(text.replace(old, new, 1))


def main():
    SOURCE.mkdir(parents=True, exist_ok=True)
    archive = subprocess.Popen(["git", "archive", "9d961601", "sim", "data"], cwd=ROOT, stdout=subprocess.PIPE)
    subprocess.run(["tar", "-x", "-C", str(SOURCE)], stdin=archive.stdout, check=True)
    if archive.wait():
        raise RuntimeError("M3 source archive failed")
    for name in ("m4_m3_counters.hpp",):
        shutil.copyfile(ROOT / "tools" / name, SOURCE / "sim/src" / name)
    for name in ("living", "craft_work", "teaching", "fire", "fire_food"):
        path = SOURCE / f"sim/src/kd/demo/{name}.cpp"
        path.write_text('#include "m4_m3_counters.hpp"\n' + path.read_text())
    patch(
        "demo/living.cpp",
        "    notice(c, h, camp);\n    l.decision_needs = needs(l);",
        "    diagnostic::Time timing(diagnostic::choosing);\n"
        "    diagnostic::hit(diagnostic::decisions);\n"
        "    notice(c, h, camp);\n"
        "    l.decision_needs = needs(l);\n"
        "    if (*std::min_element(l.decision_needs.begin(), l.decision_needs.end()) < 20 || l."
        "awake >= 129600) diagnostic::hit(diagnostic::urgent);\n"
        "    if (*std::min_element(l.decision_needs.begin(), l.decision_needs.end()) >= 60) dia"
        "gnostic::hit(diagnostic::comfortable);",
    )
    patch(
        "demo/craft_work.cpp",
        "    const auto& w = c.world();\n"
        "    const auto& raw = w.beings().raw();\n"
        "    const auto person = w.beings().id_of(h);",
        "    diagnostic::Time timing(diagnostic::reachable);\n"
        "    const auto& w = c.world();\n"
        "    const auto& raw = w.beings().raw();\n"
        "    const auto person = w.beings().id_of(h);",
    )
    patch(
        "demo/craft_work.cpp",
        "                const auto available = supply.available(id, item);",
        "                diagnostic::hit(diagnostic::input_visits);\n"
        "                if (!item.mass) diagnostic::hit(diagnostic::spent_input_visits);\n"
        "                const auto available = supply.available(id, item);",
    )
    patch(
        "demo/craft_work.cpp",
        "    Discovery::see(c, h);\n    auto& life",
        "    diagnostic::hit(diagnostic::craft_calls);\n    Discovery::see(c, h);\n    auto& life",
    )
    patch(
        "demo/craft_work.cpp",
        "        if (candidate) options.push_back(std::move(*candidate));",
        "        if (candidate) { diagnostic::hit(diagnostic::known_candidates); options.push_b"
        "ack(std::move(*candidate)); }",
    )
    patch(
        "demo/craft_work.cpp",
        "        know->hourly_draw = hour;",
        "        diagnostic::hit(diagnostic::curious_hours);\n        know->hourly_draw = hour;",
    )
    patch(
        "demo/craft_work.cpp",
        "            Candidate experiment;",
        "            diagnostic::hit(diagnostic::curious_candidates);\n            Candidate experiment;",
    )
    patch(
        "demo/craft_work.cpp",
        "    if (options.empty() || options[0].reason.score <= life.scores[life.goal]) return false;",
        "    if (seen.all.empty()) diagnostic::hit(diagnostic::empty_materials);\n"
        "    if (options.empty() || options[0].reason.score <= life.scores[life.goal]) return f"
        "alse;\n"
        "    if (options[0].reason.kind == 1) diagnostic::hit(diagnostic::selected_experiments)"
        ";",
    )
    patch(
        "demo/teaching.cpp",
        "    if (!mind || !comfortable(living, w, h, c.now())) return false;",
        "    diagnostic::hit(diagnostic::teacher_calls);\n"
        "    if (!mind || !comfortable(living, w, h, c.now())) return false;\n"
        "    diagnostic::hit(diagnostic::teacher_comfortable);",
    )
    patch(
        "demo/fire.cpp",
        "    if (!t || t->tending || raw.get<world::Work>(h).state) return false;",
        "    diagnostic::hit(diagnostic::fire_calls);\n"
        "    if (!t || t->tending || raw.get<world::Work>(h).state) return false;",
    )
    patch(
        "demo/fire.cpp",
        "    const auto f = fire(w, target);",
        "    const auto f = fire(w, target);\n    if (f.heat == 1) diagnostic::hit(diagnostic::visible_embers);",
    )
    patch(
        "demo/fire.cpp",
        "    if (!operation || score <= l.scores[l.goal]) return false;",
        "    if (operation) diagnostic::hit(diagnostic::fuel_candidates);\n"
        "    if (operation && score <= l.scores[l.goal]) diagnostic::hit(diagnostic::fire_preem"
        "pted);\n"
        "    if (!operation || score <= l.scores[l.goal]) return false;",
    )
    patch(
        "demo/fire.cpp",
        "void FireRules::deadlines(world::Context& c, ecs::Id camp) {",
        "void FireRules::deadlines(world::Context& c, ecs::Id camp) {\n"
        "    diagnostic::Time timing(diagnostic::fire_deadlines);",
    )
    patch(
        "demo/fire_food.cpp",
        "void FireRules::food_refresh(world::Context& c, ecs::Id camp) {",
        "void FireRules::food_refresh(world::Context& c, ecs::Id camp) {\n"
        "    diagnostic::Time timing(diagnostic::food_refresh);",
    )
    patch(
        "demo/fire_food.cpp",
        "        const auto& i = c.world().things().raw().get<Item>(h);",
        "        diagnostic::hit(diagnostic::food_visits);\n"
        "        const auto& i = c.world().things().raw().get<Item>(h);",
    )
    patch(
        "demo/fire_food.cpp",
        "std::optional<std::uint32_t> FireRules::cooking_recipe(const data::Catalogue& catalogue, const Item& item) {",
        "std::optional<std::uint32_t> FireRules::cooking_recipe(const data::Catalogue& catalogu"
        "e, const Item& item) {\n"
        "    diagnostic::Time timing(diagnostic::cooking_fit);",
    )
    for name in ("learning_cases", "fire_cases"):
        patch(
            f"proof/{name}.cpp",
            "    out.digest = num::to_hex(w.digests().whole);",
            "    if (progress) progress(w, w.frontier());\n    out.digest = num::to_hex(w.digests().whole);",
        )
    path = SOURCE / "sim/CMakeLists.txt"
    path.write_text(
        path.read_text() + f'\nadd_executable(m4_m3_diagnose "{ROOT}/tools/m4_m3_diagnose.cpp")\n'
        f'target_include_directories(m4_m3_diagnose PRIVATE "{ROOT}/tools")\n'
        "target_link_libraries(m4_m3_diagnose PRIVATE kd_sim)\nkindling_rules(m4_m3_diagnose)\n"
    )
    subprocess.run(
        [
            "cmake",
            "-S",
            str(SOURCE / "sim"),
            "-B",
            str(BUILD),
            "-G",
            "Ninja",
            "-DCMAKE_BUILD_TYPE=RelWithDebInfo",
            "-DKD_SIM_TESTS=OFF",
            "-DCMAKE_C_COMPILER=clang",
            "-DCMAKE_CXX_COMPILER=clang++",
            "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
            "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache",
        ],
        check=True,
    )


if __name__ == "__main__":
    main()
