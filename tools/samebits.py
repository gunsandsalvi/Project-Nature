#!/usr/bin/env python3
"""The same-bits scans (RES-05, A3.4, A17): what the compilers actually did, read from their outputs.

    python3 tools/samebits.py flags <build folder>...    each of our compile commands ends its floating-point
                                                          flags with -ffp-contract=off, uses no banned one, and
                                                          makes plain char unsigned; and the game's own build
                                                          (the extension's, with view/ code) has no test switches
    python3 tools/samebits.py scan <build folder>...     our object files hold no fused multiply-add and call no
                                                          platform maths function
    python3 tools/samebits.py same <file>...             every file of "<suite> <digest> ..." lines gives each
                                                          suite the same digest

A later -ffp-model turns fused multiply-adds back on without a warning, and an inline function compiled with them
in one file can replace the simulation's own copy at link time, so the flags and the code are both checked
(research 18). Two kinds of file are exempt from the scan: vendored code under sim/thirdparty/, since CORE-MATH asks
for its fused multiply-adds explicitly and they are exact, and the test framework's own code (sim/tests/main.cpp and
view/tests/main.cpp, which build doctest), which formats numbers with the platform's maths but never runs in the
simulation.
"""

import json
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OURS = ("sim/", "view/")
EXEMPT = ("sim/thirdparty/", "sim/tests/main.cpp", "view/tests/main.cpp")
OBJDUMP = "llvm-objdump-18"
NM = "llvm-nm-18"

# Flags that change floating-point results, in the order a compile line gives them; the last must be
# -ffp-contract=off.
FP_FLAG = re.compile(
    r"^-(ffp-contract=\S+|ffp-model=\S+|f(no-)?fast-math|Ofast|f(no-)?unsafe-math-optimizations|"
    r"f(no-)?associative-math|f(no-)?reciprocal-math|f(no-)?finite-math-only|f(no-)?signed-zeros|"
    r"f(no-)?approx-func|f(no-)?rounding-math|march=native|mrecip\S*)$"
)
BANNED_FLAG = re.compile(
    r"^-(ffast-math|Ofast|funsafe-math-optimizations|fassociative-math|freciprocal-math|ffinite-math-only|"
    r"fno-signed-zeros|fapprox-func|ffp-model=\S+|ffp-contract=(on|fast)|march=native|mrecip\S*)$"
)
# Fused multiply-add instructions on arm64 and x86-64.
FUSED = re.compile(r"\b(fmadd|fmsub|fnmadd|fnmsub|fmla|fmls|vfn?m(add|sub)\d{3}[sp][sd])\b")
# The platform's maths functions, which differ between glibc and bionic and between CPUs (research 18).
MATHS = {
    base + suffix
    for base in (
        "sin cos tan asin acos atan atan2 sinh cosh tanh asinh acosh atanh exp exp2 expm1 exp10 log log2 log10 "
        "log1p pow cbrt hypot erf erfc lgamma tgamma fmin fmax sincos"
    ).split()
    for suffix in ("", "f", "l")
}


def relative(path):
    return os.path.relpath(os.path.abspath(path), ROOT).replace(os.sep, "/")


def compile_commands(build):
    with open(os.path.join(build, "compile_commands.json")) as f:
        return json.load(f)


def arguments(entry):
    if "arguments" in entry:
        return entry["arguments"]
    return entry["command"].split()


def check_flags(builds):
    """Problems with the floating-point flags of our compile commands in these build folders, and test switches
    compiled into the game's own build (RES-10)."""
    problems, seen = [], 0
    for build in builds:
        commands = compile_commands(build)
        game = any(relative(os.path.join(e["directory"], e["file"])).startswith("view/") for e in commands)
        for entry in commands:
            path = relative(os.path.join(entry["directory"], entry["file"]))
            if not path.startswith(OURS):
                continue
            seen += 1
            if game and "-DKD_TEST_SWITCHES" in arguments(entry):
                problems.append(f"{build}: {path} is compiled with the test switches, which the game never has")
            fp = [a for a in arguments(entry) if FP_FLAG.match(a)]
            banned = [a for a in fp if BANNED_FLAG.match(a)]
            if banned:
                problems.append(f"{build}: {path} is compiled with {' '.join(banned)}")
            if not fp or fp[-1] != "-ffp-contract=off":
                last = fp[-1] if fp else "nothing"
                problems.append(f"{build}: {path}'s last floating-point flag is {last}, not -ffp-contract=off")
            signs = [a for a in arguments(entry) if a in ("-funsigned-char", "-fsigned-char", "-fno-unsigned-char")]
            if not signs or signs[-1] != "-funsigned-char":
                problems.append(f"{build}: {path} is compiled without -funsigned-char, so plain char's sign differs")
    return problems, seen


def our_objects(build):
    """Our own object files in a build folder: sim/ and view/ code, never the exempt files or godot-cpp."""
    out = []
    for entry in compile_commands(build):
        source = relative(os.path.join(entry["directory"], entry["file"]))
        if not source.startswith(OURS) or source.startswith(EXEMPT):
            continue
        args = arguments(entry)
        if "-o" in args:
            out.append(os.path.join(entry["directory"], args[args.index("-o") + 1]))
    return out


def scan(builds):
    """Problems in our object files: fused multiply-adds, and calls to the platform's maths functions."""
    problems, scanned = [], 0
    for build in builds:
        for obj in our_objects(build):
            if not os.path.exists(obj):
                problems.append(f"{obj} is missing: build {build} first")
                continue
            scanned += 1
            dump = subprocess.run([OBJDUMP, "-d", "--no-show-raw-insn", obj], capture_output=True, text=True)
            fused = sorted({m.group(1) for m in FUSED.finditer(dump.stdout)})
            if fused:
                problems.append(f"{relative(obj)}: fused multiply-add instructions ({', '.join(fused)})")
            names = subprocess.run([NM, "-u", "--just-symbol-name", obj], capture_output=True, text=True)
            called = sorted(set(names.stdout.split()) & MATHS)
            if called:
                problems.append(f"{relative(obj)}: calls the platform's maths ({', '.join(called)})")
    return problems, scanned


def digests(path):
    out = {}
    with open(path) as f:
        for line in f:
            parts = line.split()
            if len(parts) >= 2:
                out[parts[0]] = parts[1]
    return out


def same(paths):
    """Problems where the files' suites differ: each suite's digest must be the same in every file."""
    runs = {p: digests(p) for p in paths}
    problems = []
    suites = sorted({s for d in runs.values() for s in d})
    for suite in suites:
        values = {p: d.get(suite, "missing") for p, d in runs.items()}
        if len(set(values.values())) != 1:
            listed = "; ".join(f"{os.path.basename(p)} {v}" for p, v in values.items())
            problems.append(f"suite {suite} differs: {listed}")
    return problems, len(suites)


def main(argv):
    if len(argv) < 2 or argv[0] not in ("flags", "scan", "same"):
        print(__doc__)
        return 2
    command, args = argv[0], argv[1:]
    if command == "flags":
        problems, n = check_flags(args)
        ok = f"Same bits: the flags of {n} compile commands end with -ffp-contract=off, char unsigned"
    elif command == "scan":
        problems, n = scan(args)
        ok = f"Same bits: {n} object files with no fused multiply-add and no platform maths"
    else:
        problems, n = same(args)
        ok = f"Same bits: {n} suites with one digest in all {len(args)} runs"
    for p in problems:
        print(f"Same bits: {p}")
    print(ok if not problems else f"Same bits: FAIL ({len(problems)} problems)")
    return 0 if not problems else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
