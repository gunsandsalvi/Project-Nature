"""The kill test (PLT-07, TIM-05, A3.7): the kindling tool keeps a crowd's world in a folder and is killed at 100
random moments, as a phone kills an app; each time it is started again it must open the world and carry on, and the
world it ends with must be the one an unbroken run ends with, its history made again the same as it was written.

    python3 tools/killtest.py <kindling> <data folder> [kills]
"""

import random
import shutil
import subprocess
import sys
import tempfile
import time

# A crowd of 100 camps over four game days, a snapshot every three game hours, and three camps called home.
ARGS = [
    "--camps", "100", "--until", str(4 * 86400), "--every", str(3 * 3600),
    "--call", "36000:3", "--call", "120600:7", "--call", "313200:12",
]  # fmt: skip


def last_line(out: str) -> str:
    lines = [line for line in out.splitlines() if line.strip()]
    return lines[-1] if lines else ""


def main() -> int:
    tool, data = sys.argv[1], sys.argv[2]
    kills = int(sys.argv[3]) if len(sys.argv) > 3 else 100
    args = [*ARGS, "--data", data]
    with tempfile.TemporaryDirectory() as tmp:
        started = time.monotonic()
        unbroken = subprocess.run([tool, "keep", f"{tmp}/unbroken", *args], capture_output=True, text=True)
        took = time.monotonic() - started
        if unbroken.returncode != 0:
            print(f"Kill test: the unbroken run failed\n{unbroken.stdout}{unbroken.stderr}")
            return 1
        want = last_line(unbroken.stdout)

        # how long a start takes, to open the world and carry on a little
        started = time.monotonic()
        subprocess.run([tool, "keep", f"{tmp}/unbroken", *args], capture_output=True, text=True)
        startup = time.monotonic() - started

        # killed at moments chosen at random, each start getting about a fiftieth of a run beyond its own start, so
        # the kills fall all through the run, its starts and its snapshots
        folder = f"{tmp}/killed"
        rng = random.Random(20401)
        opened = 0
        finished = 0
        for _ in range(kills):
            p = subprocess.Popen([tool, "keep", folder, *args], stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                 text=True)  # fmt: skip
            time.sleep(rng.uniform(0.0, 2.0 * (startup + took / 50)))
            p.kill()
            out, err = p.communicate()
            first = out.splitlines()[0] if out.strip() else ""
            if first.startswith(("made", "opened")):
                opened += 1
            elif first and not first.startswith("damaged"):
                print(f"Kill test: a start failed: {first}\n{err}")
                return 1
            if p.returncode == 0:
                # it ran to the end before its kill: the world begins again, so the kills go on mid-run
                finished += 1
                if last_line(out).split(" snapshots ")[0] != want.split(" snapshots ")[0]:
                    print(f"Kill test: FAIL\n  unbroken: {want}\n  killed:   {last_line(out)}")
                    return 1
                shutil.rmtree(folder)
        final = subprocess.run([tool, "keep", folder, *args], capture_output=True, text=True)
        got = last_line(final.stdout)
        if final.returncode != 0 or got.split(" snapshots ")[0] != want.split(" snapshots ")[0]:
            print(f"Kill test: FAIL\n  unbroken: {want}\n  killed:   {got}\n{final.stdout}{final.stderr}")
            return 1
        print(
            f"Kill test: {kills} kills, {opened} after the world opened, {finished} too late to stop a run; the world "
            f"ends as the unbroken run's, {got.split(' snapshots ')[0]}"
        )
        return 0


if __name__ == "__main__":
    sys.exit(main())
