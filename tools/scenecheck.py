"""The scenes' checks before work joins (PRC-10, A17), run with the kindling tool:
- every scene in data/scenes passes its rule with no oddity (RES-21, RES-09);
- the planted scene flags each oddity planted in it, and nothing else, and each run's world names its switch (RES-12,
  RES-10);
- a scene whose rule fails on its runs runs as many again on fresh seeds and is judged on all of them (RES-13);
- the repeat check (RES-05, PLT-05): one scene and one benchmark world each run twice, once on one core and once on four
  with a stop and resume between, and must end identical.

    python3 tools/scenecheck.py <kindling> <data folder>
"""

import json
import os
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCENES = ROOT / "data" / "scenes"
TEST_SCENES = ROOT / "sim" / "tests" / "scenes"
# The scene the repeat check runs twice.
REPEATED = "greetings"
# The benchmark world (A18.1): 10,000 markers in 400 camps over two game days, a checkpoint every three game hours and
# three camps called home; on four cores, in islands of one-minute windows (A3.3).
WORLD = [
    "--camps", "400", "--until", str(2 * 86400), "--every", str(3 * 3600),
    "--call", "36000:3", "--call", "100800:7", "--call", "151200:12",
]  # fmt: skip
FOUR_CORES = ["--islands", "60", "--threads", "4"]
# Each switch the planted scene plants, and the oddity it must be flagged by, in its words.
PLANTED = {
    "plant_wander": r"day \d+: a marker \d+ m from its camp, farther than 5000",
    "plant_insomnia": r"day \d+: a marker awake at midnight 2 nights running",
    "plant_chatter": r"greetings_per_camp_day \d+, outside its expected 5 to 25",
    "plant_crash": r"it crashed, stopped by signal \d+",
    "plant_leak": r"its memory crept up by \d+ MB over its run",
    "plant_bad_save": r"its last save did not open again as the world was",
}


class Failed(Exception):
    pass


def last_line(out: str) -> str:
    lines = [line for line in out.splitlines() if line.strip()]
    return lines[-1] if lines else ""


def scene(tool: str, path: Path, out: str, data: str, *more: str) -> tuple[subprocess.CompletedProcess, dict]:
    """A scene run by the tool, and its report."""
    command = [tool, "scene", str(path), "--out", out, "--data", data, *more]
    done = subprocess.run(command, capture_output=True, text=True)
    report = Path(out) / "report.json"
    if not report.is_file():
        raise Failed(f"{path.name} left no report\n{done.stdout}{done.stderr}")
    return done, json.loads(report.read_text())


def newest_snapshot(folder: Path) -> int:
    """The game second of the newest snapshot in a world's folder, or -1."""
    names = [p.name for p in (folder / "snapshots").glob("*.kds")] if (folder / "snapshots").is_dir() else []
    return max((int(n[:-4]) for n in names if n[:-4].isdigit()), default=-1)


def kept(folder: Path) -> dict[str, bytes]:
    """What a world's folder keeps that a stop must not change: its name, history and journal, and a scene run's samples
    and result; not its snapshots, which hold the real time played."""
    names = ["world.toml", "journal.log", "days.txt", "result.txt"]
    files = {n: (folder / n).read_bytes() for n in names if (folder / n).is_file()}
    for f in sorted((folder / "history").glob("*")):
        files[f"history/{f.name}"] = f.read_bytes()
    return files


def differs(one: Path, four: Path) -> str:
    """The first file kept differently in two folders, or nothing."""
    a, b = kept(one), kept(four)
    for name in sorted(set(a) | set(b)):
        if a.get(name) != b.get(name):
            return f"{four.name}/{name}"
    return ""


def stopped(command: list[str], ready) -> None:
    """The command started in a session of its own, and killed with everything it started once ready() holds."""
    p = subprocess.Popen(command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, start_new_session=True)
    deadline = time.monotonic() + 300
    while p.poll() is None and not ready() and time.monotonic() < deadline:
        time.sleep(0.002)
    ended = p.poll() is not None
    if not ended:
        os.killpg(p.pid, signal.SIGKILL)
    p.wait()
    if ended:
        raise Failed(f"{' '.join(command[:3])} ended before it could be stopped")


# checks: RES-21 RES-09
def every_scene(tool: str, data: str, tmp: str) -> list[str]:
    lines = []
    for path in sorted(SCENES.glob("*.toml")):
        if path.stem == REPEATED:
            continue  # the repeat check runs it, twice
        done, report = scene(tool, path, f"{tmp}/{path.stem}", data, "--fresh")
        if done.returncode != 0:
            raise Failed(f"{path.name} failed\n{done.stdout}{done.stderr}")
        lines.append(last_line(done.stdout))
    return lines


# checks: RES-12 RES-10
def planted(tool: str, data: str, tmp: str) -> str:
    out = f"{tmp}/planted"
    done, report = scene(tool, TEST_SCENES / "planted.toml", out, data, "--fresh")
    if done.returncode == 0:
        raise Failed(f"the planted scene passed, its oddities unseen\n{done.stdout}")
    seen = set()
    for run in report["each"]:
        switches, oddities = run["switches"], run["oddities"]
        if len(switches) != 1 or switches[0] not in PLANTED:
            raise Failed(f"planted run {run['index']} has switches {switches}, not one planted fault")
        switch = switches[0]
        if len(oddities) != 1 or not re.fullmatch(PLANTED[switch], oddities[0]):
            raise Failed(f"planted run {run['index']} with {switch} flagged {oddities}, not its one oddity")
        world = (Path(out) / f"run-{run['index']:03d}" / "world.toml").read_text()
        if "test = true" not in world or f'switches = ["{switch}"]' not in world:
            raise Failed(f"planted run {run['index']}'s world.toml does not mark it a test world with {switch}")
        seen.add(switch)
    if seen != set(PLANTED):
        raise Failed(f"the planted scene planted {sorted(seen)}, not each of {sorted(PLANTED)}")
    return f"each of the {len(seen)} planted oddities flagged, and nothing else; each world names its switch"


# checks: RES-13
def rerun(tool: str, data: str, tmp: str) -> str:
    done, r = scene(tool, TEST_SCENES / "rerun.toml", f"{tmp}/rerun", data, "--fresh")
    runs = r["runs"]
    each = r["each"]
    values = [e["measures"].get(r["measure"]) for e in each]
    met = [
        v is not None and (r["at_least"] is None or v >= r["at_least"]) and (r["at_most"] is None or v <= r["at_most"])
        for v in values
    ]
    first = sum(met[:runs])
    # the rule's count scaled to twice the runs, rounded against passing
    needed = (r["in"] * 2 * runs + runs - 1) // runs
    if first >= r["in"]:
        raise Failed(f"the rerun scene passed on its first {runs} runs, so it shows no rerun")
    seeds = [e["seed"] for e in each]
    if not r["reran"] or r["judged"] != 2 * runs or seeds != list(range(r["seed"], r["seed"] + 2 * runs)):
        raise Failed(f"the rerun scene was not judged on {2 * runs} runs of fresh seeds: {r['judged']}, {seeds}")
    if r["passes"] != sum(met) or r["needed"] != needed or r["passed"] != (sum(met) >= needed):
        raise Failed(f"the rerun scene's verdict is not its runs': {r['passes']} of {r['judged']}, {r['needed']}")
    if (done.returncode == 0) != r["passed"]:
        raise Failed(f"the rerun scene's exit {done.returncode} is not its verdict")
    verdict = "passes" if r["passed"] else "fails"
    return (
        f"a rule failed by {first} of {runs} runs ran {runs} more and {verdict} by {sum(met)} of {2 * runs}, "
        f"{needed} needed"
    )


# checks: RES-05 PLT-05 PRC-10
def repeat_scene(tool: str, data: str, tmp: str) -> str:
    path = SCENES / f"{REPEATED}.toml"
    one_done, one = scene(tool, path, f"{tmp}/one", data, "--jobs", "1", "--fresh")
    if one_done.returncode != 0:
        raise Failed(f"{path.name} failed on one core\n{one_done.stdout}{one_done.stderr}")
    # on four cores, stopped once some runs are done and another is days into its world, then resumed
    out = Path(f"{tmp}/four")
    until = one["until"]

    def runs() -> list[Path]:
        return sorted(out.glob("run-*")) if out.is_dir() else []

    def ready() -> bool:
        folders = runs()
        done = sum((f / "result.txt").is_file() for f in folders)
        midway = any(not (f / "result.txt").is_file() and 3 * 86400 <= newest_snapshot(f) < until for f in folders)
        return done >= 3 and midway

    stopped([tool, "scene", str(path), "--out", str(out), "--jobs", "4", "--data", data, "--fresh"], ready)
    done_then = sum((f / "result.txt").is_file() for f in runs())
    midway = sum(not (f / "result.txt").is_file() and newest_snapshot(f) > 0 for f in runs())
    four_done, four = scene(tool, path, str(out), data, "--jobs", "4")
    if four_done.returncode != 0:
        raise Failed(f"{path.name} failed on four cores after its stop\n{four_done.stdout}{four_done.stderr}")
    for report in (one, four):
        report.pop("seconds")
    if one != four:
        for a, b in zip(one["each"], four["each"], strict=False):
            if a != b:
                raise Failed(f"{path.name} differs on four cores after its stop:\n  one:  {a}\n  four: {b}")
        raise Failed(f"{path.name}'s report differs on four cores after its stop")
    for folder in runs():
        if name := differs(Path(f"{tmp}/one") / folder.name, folder):
            raise Failed(f"{path.name} kept {name} differently on four cores after its stop")
    return (
        f"{path.stem}: {len(one['each'])} runs identical on one core and on four, stopped with {done_then} done and "
        f"{midway} part-way, then resumed"
    )


# checks: RES-05 PLT-05 PRC-10
def repeat_world(tool: str, data: str, tmp: str) -> str:
    one = subprocess.run([tool, "keep", f"{tmp}/w1", *WORLD, "--data", data], capture_output=True, text=True)
    if one.returncode != 0:
        raise Failed(f"the benchmark world failed on one core\n{one.stdout}{one.stderr}")
    want = last_line(one.stdout).split(" snapshots ")[0]
    folder = Path(f"{tmp}/w4")
    four = [tool, "keep", str(folder), *WORLD, *FOUR_CORES, "--data", data]
    stopped(four, lambda: newest_snapshot(folder) >= 86400)
    stop = newest_snapshot(folder)
    resumed = subprocess.run(four, capture_output=True, text=True)
    if resumed.returncode != 0 or not resumed.stdout.startswith("opened"):
        raise Failed(f"the benchmark world did not resume on four cores\n{resumed.stdout}{resumed.stderr}")
    got = last_line(resumed.stdout).split(" snapshots ")[0]
    if got != want:
        raise Failed(f"the benchmark world differs on four cores after its stop:\n  one:  {want}\n  four: {got}")
    if name := differs(Path(f"{tmp}/w1"), folder):
        raise Failed(f"the benchmark world kept {name} differently on four cores after its stop")
    return f"the benchmark world identical on one core and on four, stopped at game second {stop}: {got}"


def main() -> int:
    tool, data = sys.argv[1], sys.argv[2]
    tmp = tempfile.mkdtemp()
    try:
        lines = [
            *every_scene(tool, data, tmp),
            planted(tool, data, tmp),
            rerun(tool, data, tmp),
            repeat_scene(tool, data, tmp),
            repeat_world(tool, data, tmp),
        ]
    except Failed as e:
        print(f"Scenes: FAIL: {e}")
        return 1
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    print("Scenes:")
    for line in lines:
        print(f"  {line}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
