#!/usr/bin/env python3
"""What the C++ checks may skip (PRC-10, A17), so a check costs about what changed.

clang-tidy 18 runs only on the files whose code, the headers they read, their compile commands or the lint rules
changed since they last passed, on every core at once; and a C++ project's tests are skipped while nothing they are
built from has changed since they passed. What a file or a project depends on is what the compiler told ninja as it
built it, so a changed header counts for every file that reads it.

    python3 tools/cppcache.py lint <build folder> <header filter> <file>...
    python3 tools/cppcache.py tests <build folder>     # prints the fingerprint of what the tests are built from

The lint's passes are kept as empty files named by their fingerprints in <build folder>/tidy-passed; tools/check.sh
keeps the tests' last passing fingerprint in <build folder>/tests.passed.
"""

import concurrent.futures
import hashlib
import json
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def run(args):
    return subprocess.run(args, capture_output=True, text=True, check=False)


def contents(path):
    try:
        with open(path, "rb") as f:
            return f.read()
    except OSError:
        return b"(missing)"


def fingerprint(parts):
    """A SHA-256 of named parts, each a name and its bytes, in the order given."""
    h = hashlib.sha256()
    for name, data in parts:
        h.update(name.encode())
        h.update(b"\0")
        h.update(data)
        h.update(b"\0")
    return h.hexdigest()


def configs(path, root=ROOT):
    """Every .clang-tidy from the file's folder up to the repository's root, nearest first."""
    found = []
    folder = os.path.dirname(os.path.abspath(path))
    while True:
        config = os.path.join(folder, ".clang-tidy")
        if os.path.isfile(config):
            found.append(config)
        if folder == root or folder == os.path.dirname(folder):
            return found
        folder = os.path.dirname(folder)


def depends(build, output):
    """The files an object was compiled from, as the compiler told ninja: its source and every header it read; None
    when ninja has no record of them."""
    lines = run(["ninja", "-C", build, "-t", "deps", output]).stdout.splitlines()
    if not lines or "(VALID)" not in lines[0]:
        return None
    return [os.path.normpath(os.path.join(build, line.strip())) for line in lines[1:] if line.strip()]


def lint_key(version, header_filter, command, config_files, dependencies):
    """A file's lint fingerprint: the linter, its header filter, the compile command, the rules and every file read."""
    parts = [("version", version), ("filter", header_filter.encode()), ("command", command.encode())]
    parts += [(c, contents(c)) for c in config_files]
    parts += [(d, contents(d)) for d in dependencies]
    return fingerprint(parts)


def lint(build, header_filter, files):
    with open(os.path.join(build, "compile_commands.json")) as f:
        entries = {os.path.realpath(e["file"]): e for e in json.load(f)}
    version = run(["clang-tidy-18", "--version"]).stdout.encode()
    passed = os.path.join(build, "tidy-passed")
    os.makedirs(passed, exist_ok=True)
    todo = []
    for path in files:
        entry = entries.get(os.path.realpath(path))
        deps = depends(build, entry["output"]) if entry else None
        if deps is None:
            todo.append((path, None))
            continue
        stamp = os.path.join(passed, lint_key(version, header_filter, entry["command"], configs(path), deps))
        if not os.path.exists(stamp):
            todo.append((path, stamp))

    def one(item):
        path, stamp = item
        done = run(["clang-tidy-18", "-p", build, "--quiet", f"--header-filter={header_filter}", path])
        if done.returncode != 0:
            return f"{path}:\n{done.stdout}{done.stderr}"
        if stamp:
            with open(stamp, "w"):
                pass
        return None

    with concurrent.futures.ThreadPoolExecutor(os.cpu_count() or 1) as pool:
        failures = [f for f in pool.map(one, todo) if f]
    if failures:
        print("\n".join(failures))
        return 1
    print(f"{len(todo)} linted, {len(files) - len(todo)} unchanged since they passed")
    return 0


# What the tests read from the repository as they run, not as they are built (KD_REPO in sim/tests): the game's
# catalogue, which the old saves' corpus opens with, and the corpus itself. A change to either alone runs them again.
READ_AS_THEY_RUN = ("data", "sim/tests/corpus")


def tests(build):
    """The fingerprint of everything a project's tests are built from or read as they run: the build's own rules,
    every object's source and headers, the test commands, the files they name (such as arm64.sh and the sources it
    compiles), the folders in READ_AS_THEY_RUN, and the compilers and emulator. "unknown" when ninja has no record of
    an object, so the tests run."""
    with open(os.path.join(build, "compile_commands.json")) as f:
        entries = json.load(f)
    files = set()
    for entry in entries:
        deps = depends(build, entry["output"])
        if deps is None:
            return "unknown"
        files.update(deps)
    tests_text = b""
    for folder, _, names in sorted(os.walk(build)):
        if "CTestTestfile.cmake" in names:
            text = contents(os.path.join(folder, "CTestTestfile.cmake"))
            tests_text += text
            for token in re.findall(rb'"([^"]+)"', text):
                name = token.decode(errors="replace")
                if os.path.isfile(name):
                    files.add(os.path.normpath(name))
    for top in READ_AS_THEY_RUN:
        for folder, _, names in os.walk(os.path.join(ROOT, top)):
            files.update(os.path.join(folder, name) for name in names)
    versions = "".join(
        run([tool, "--version"]).stdout for tool in ("c++", "aarch64-linux-gnu-g++", "qemu-aarch64-static")
    )
    parts = [("versions", versions.encode()), ("ninja", contents(os.path.join(build, "build.ninja")))]
    parts += [("tests", tests_text)]
    parts += [(name, contents(name)) for name in sorted(files)]
    return fingerprint(parts)


def main(argv):
    if len(argv) >= 4 and argv[1] == "lint":
        return lint(argv[2], argv[3], argv[4:])
    if len(argv) == 3 and argv[1] == "tests":
        print(tests(argv[2]))
        return 0
    print(__doc__.strip().split("\n\n")[2], file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv))
