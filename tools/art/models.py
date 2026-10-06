#!/usr/bin/env python3
"""A family of the kit's parts made and shown (IMPLEMENTATION.md, the art lane): its Blender file built from its
script, every part checked, drawn and put on its preview sheet.

    python3 tools/art/models.py <family>... [--root DIR] [--no-build] [--no-sheet]

For each family (camp, plants, rocks, people, deer), in art/models/ under the root:
1. <family>.py, run in Blender headless, builds <family>.blend (skipped with --no-build, or when there is no
   script, as for a file the owner changed by hand in Blender);
2. partcheck.py measures every part and writes <family>-stretch.txt, the stretch report;
3. preview.py draws every part at true size under xvfb-run, in a scratch folder (skipped with --no-sheet);
4. partsheet.py puts the drawings on <family>-sheet.webp, the preview sheet;
5. where <family>-textures.json names the materials its roles wear, preview.py draws them again wearing their
   textures, on <family>-textured.webp.
Blender is `blender` unless BLENDER names another. Prints each step's last lines; exits 1 if any part fails.

Implements PRE-46, see A6.1 and A6.5.
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))


def blender():
    return os.environ.get("BLENDER", "blender")


def run(args, show=4):
    """Runs a command; returns its exit status, printing the last lines of what it wrote."""
    done = subprocess.run(args, capture_output=True, text=True)
    lines = [x for x in (done.stdout + done.stderr).splitlines() if x.strip()]
    for line in lines[-show:]:
        print("   " + line)
    return done.returncode


def make(family, root=ROOT, build=True, sheet=True):
    """Builds, checks and draws one family; returns 0 when every part passes."""
    folder = os.path.join(root, "art", "models")
    script = os.path.join(folder, f"{family}.py")
    blend = os.path.join(folder, f"{family}.blend")
    print(f"== {family}")
    if build and os.path.exists(script):
        code = run([blender(), "-b", "--factory-startup", "--python-exit-code", "1", "--python", script, "--", blend])
        if code != 0 or not os.path.exists(blend):
            print(f"   {family}.py failed ({code})")
            return 1
    if not os.path.exists(blend):
        print(f"   no {family}.blend")
        return 1
    report = os.path.join(folder, f"{family}-stretch.txt")
    with tempfile.TemporaryDirectory() as scratch:
        check = os.path.join(scratch, "check.json")
        status = run(
            [
                blender(),
                "-b",
                blend,
                "--python",
                os.path.join(HERE, "partcheck.py"),
                "--",
                "--report",
                report,
                "--json",
                check,
            ],
            show=1,
        )
        if sheet:
            views = os.path.join(scratch, "views")
            code = run(
                ["xvfb-run", "-a", blender(), "-b", blend, "--python", os.path.join(HERE, "preview.py"), "--", views],
                show=1,
            )
            if code != 0:
                print(f"   preview.py failed ({code})")
                return 1
            out = os.path.join(folder, f"{family}-sheet.webp")
            code = run([sys.executable, os.path.join(HERE, "partsheet.py"), views, check, out], show=1)
            if code != 0:
                return 1
            chosen = os.path.join(folder, f"{family}-textures.json")
            if os.path.exists(chosen):  # the parts again, wearing their textures
                views = os.path.join(scratch, "textured")
                preview = [blender(), "-b", blend, "--python", os.path.join(HERE, "preview.py"), "--", views]
                code = run(["xvfb-run", "-a", *preview, "--textures", chosen], show=1)
                out = os.path.join(folder, f"{family}-textured.webp")
                textured = [sys.executable, os.path.join(HERE, "partsheet.py"), views, check, out, "--textured"]
                if code != 0 or run(textured, show=1):
                    print("   the textured sheet failed")
                    return 1
    for f in os.listdir(folder):  # Blender's copies of a file it saved over, never kept
        if f.endswith(".blend1"):
            os.remove(os.path.join(folder, f))
    return 1 if status else 0


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("families", nargs="+")
    ap.add_argument("--root", default=ROOT)
    ap.add_argument("--no-build", action="store_true")
    ap.add_argument("--no-sheet", action="store_true")
    args = ap.parse_args(argv)
    if shutil.which(blender()) is None:
        print(f"no Blender at {blender()!r}")
        return 2
    failed = [f for f in args.families if make(f, args.root, not args.no_build, not args.no_sheet)]
    print(
        f"{len(args.families)} families, {len(failed)} with a failing part"
        + (f": {', '.join(failed)}" if failed else "")
    )
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
