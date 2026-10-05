#!/usr/bin/env python3
"""Vendors CORE-MATH's correctly rounded maths functions into sim/thirdparty/core-math/ (RES-05, A3.4).

    python3 tools/core-math.py <a CORE-MATH checkout at the pinned commit>

It copies each function's C file and the headers it includes, keeping upstream's folders, with the licence, and writes
hard-cases.inc: an even sample of the inputs CORE-MATH lists as the hardest to round for each function (its .wc file),
which the oracle test and the numbers' proof suite run. The functions are kept here because their host is the one
source a cloud session might not reach (A2.4); to update them, change COMMIT, run this on a checkout of the new commit,
and let the oracle test check every function again.
"""

import os
import re
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "sim", "thirdparty", "core-math")
COMMIT = "e072473ef919f1884b8cd0ac96817f068df793f7"
# The functions the simulation uses, and how many arguments each takes: angles are turns (A3.4), so the
# trigonometric functions are the ones in half turns, sin(pi x) and asin(x) / pi.
FUNCTIONS = {
    "cbrt": 1,
    "exp": 1,
    "exp2": 1,
    "expm1": 1,
    "log": 1,
    "log2": 1,
    "log1p": 1,
    "pow": 2,
    "tanh": 1,
    "erf": 1,
    "hypot": 2,
    "sinpi": 1,
    "cospi": 1,
    "tanpi": 1,
    "asinpi": 1,
    "acospi": 1,
    "atanpi": 1,
    "atan2pi": 2,
}
SAMPLE = 256
INCLUDE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.M)


def number(text):
    """A value as the .wc files write it ("0x1.8p-3", "+0x.fp-1022", "-0"), or None if not finite or not a number
    (a few lines of atan2pi's file carry the remains of a log message)."""
    text = text.strip()
    try:
        value = float.fromhex(text) if "0x" in text else float(text)
    except (ValueError, OverflowError):
        return None
    return value if abs(value) != float("inf") and value == value else None


def cases(path, arity):
    """The finite cases of a .wc file, in its order: tuples of `arity` floats."""
    out = []
    with open(path) as f:
        for line in f:
            line = line.split("#", 1)[0].strip()
            if not line:
                continue
            values = [number(v) for v in line.split(",")]
            if len(values) == arity and all(v is not None for v in values):
                out.append(tuple(values))
    return out


def sample(items, n):
    """n items spread evenly through the list, always the same ones."""
    if len(items) <= n:
        return list(items)
    return [items[i * len(items) // n] for i in range(n)]


def literal(value):
    """A C++ hexadecimal literal for a double, which every compiler reads exactly, the sign of a zero included."""
    return value.hex()


def headers(folder, name, seen):
    """The local headers a file includes, and theirs, in order."""
    with open(os.path.join(folder, name)) as f:
        for header in INCLUDE.findall(f.read()):
            if header not in seen and os.path.exists(os.path.join(folder, header)):
                seen.append(header)
                headers(folder, header, seen)
    return seen


def main(argv):
    if len(argv) != 1:
        print(__doc__)
        return 2
    source = argv[0]
    head = subprocess.run(["git", "-C", source, "rev-parse", "HEAD"], capture_output=True, text=True).stdout.strip()
    if head != COMMIT:
        print(f"CORE-MATH: the checkout is at {head or 'no commit'}, not {COMMIT}")
        return 1
    shutil.rmtree(os.path.join(OUT, "src"), ignore_errors=True)
    shutil.copy(os.path.join(source, "LICENSE"), os.path.join(OUT, "LICENSE"))
    lines = [
        f"// Hard cases for CORE-MATH's functions: {SAMPLE} inputs a function, spread evenly through the finite ones",
        f"// its .wc file lists as the hardest to round, at commit {COMMIT[:8]}. Written by tools/core-math.py.",
        "// Each line is KD_HARD(function, x, y), y being 0 for a function of one argument.",
    ]
    for name, arity in FUNCTIONS.items():
        folder = os.path.join(source, "src", "binary64", name)
        target = os.path.join(OUT, "src", "binary64", name)
        os.makedirs(target)
        for f in [f"{name}.c", *headers(folder, f"{name}.c", [])]:
            shutil.copy(os.path.join(folder, f), os.path.join(target, f))
        for values in sample(cases(os.path.join(folder, f"{name}.wc"), arity), SAMPLE):
            x, y = (values + (0.0,))[:2]
            lines.append(f"KD_HARD({name}, {literal(x)}, {literal(y)})")
    with open(os.path.join(OUT, "hard-cases.inc"), "w") as f:
        f.write("\n".join(lines) + "\n")
    print(f"CORE-MATH: {len(FUNCTIONS)} functions and their hard cases vendored from {COMMIT[:8]}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
