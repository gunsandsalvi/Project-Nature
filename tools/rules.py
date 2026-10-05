#!/usr/bin/env python3
"""The banned list (RES-05, PRC-10, A3.4): what the simulation must never use, read from the code's syntax tree by
clang-query 18, each rule saying what to use instead.

    python3 tools/rules.py check <build folder> <file>...   the rules over these files of sim/src and view/src, as the
                                                            build folder compiles them; files unchanged since they
                                                            passed are skipped
    python3 tools/rules.py list                             the rules and what to use instead

A rule applies to the folders it names, and a few files are exempt from one rule each, for the reason given: the
one place that does what the rule forbids everywhere else. Plain char's sign, which differs between x86-64 and
arm64, is not a rule here: every build makes char unsigned (-funsigned-char, A2.2), and tools/samebits.py checks
the flag on every compile command.
"""

import concurrent.futures
import json
import os
import re
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cppcache  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
QUERY = "clang-query-18"
SIM = ("sim/src/",)
BOTH = ("sim/src/", "view/src/")
STD = r"::std::(__1::)?"
MATHS = (
    "sin|cos|tan|asin|acos|atan|atan2|sinh|cosh|tanh|asinh|acosh|atanh|exp|exp2|expm1|exp10|log|log2|log10|"
    "log1p|pow|cbrt|hypot|erf|erfc|lgamma|tgamma|sincos|sinpi|cospi|tanpi|asinpi|acospi|atanpi|atan2pi"
)
# What may change the outcome of an expression: a call to a member that is not const, other than the containers'
# accessors, which change nothing; or ++ and --. A lambda is only made where it is written, so what its body does is
# not counted.
ACCESSORS = (
    '"begin", "end", "rbegin", "rend", "data", "front", "back", "at", "get", "value", "find", "lower_bound", '
    '"upper_bound", "equal_range", "operator[]", "operator*", "operator->"'
)
CHANGES = f"cxxMemberCallExpr(callee(cxxMethodDecl(unless(isConst()), unless(hasAnyName({ACCESSORS})))))"
EFFECT = (
    f"expr(unless(lambdaExpr()), anyOf({CHANGES}, hasDescendant({CHANGES}),"
    'unaryOperator(hasAnyOperatorName("++", "--")),'
    'hasDescendant(unaryOperator(hasAnyOperatorName("++", "--")))))'
)
PAIRS = [(0, 1), (0, 2), (1, 2), (0, 3), (1, 3), (2, 3)]
# Operators whose operands C++17 evaluates in a fixed order, or one of which may not be evaluated at all.
SEQUENCED = '"&&", "||", ",", "<<", ">>", "=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="'


def type_named(pattern):
    """A use of a type (in a declaration, a cast, a template argument) whose declaration's name matches."""
    return f'typeLoc(loc(qualType(hasDeclaration(namedDecl(matchesName("{pattern}"))))))'


def calls(pattern):
    """A call of a function whose qualified name matches."""
    return f'callExpr(callee(functionDecl(matchesName("{pattern}"))))'


# Each rule: its name, the folders it applies to, its matchers, what to use instead, and the files exempt from it.
RULES = [
    {
        "name": "long-double",
        "where": BOTH,
        "match": [
            'declaratorDecl(hasType(asString("long double")))',
            'expr(hasType(asString("long double")))',
        ],
        "instead": "long double is 80 bits on x86-64 and 128 on arm64: use double",
    },
    {
        "name": "float",
        "where": SIM,
        "match": ['declaratorDecl(hasType(asString("float")))', 'expr(hasType(asString("float")))'],
        "instead": "use double for working values and whole numbers in state (A3.4)",
    },
    {
        "name": "platform-maths",
        "where": BOTH,
        "match": [calls(f"^::(std::)?(__builtin_)?({MATHS})[fl]?$")],
        "instead": "the platform's maths differs between the cloud and the phone: use kd::num's (kd/num/maths.hpp)",
    },
    {
        "name": "fmin-fmax",
        "where": BOTH,
        "match": [calls("^::(std::)?(__builtin_)?f(min|max)[fl]?$")],
        "instead": "fmin and fmax treat signed zeros differently on each chip: use std::min or std::max",
    },
    {
        "name": "float-to-int",
        "where": SIM,
        "match": ['castExpr(hasCastKind("CK_FloatingToIntegral"))'],
        "instead": "a cast out of range differs between chips: use kd::num::to_int (kd/num/convert.hpp)",
        "exempt": {"sim/src/kd/num/convert.cpp": "the one checked conversion"},
    },
    {
        "name": "float-text",
        "where": SIM,
        "match": [
            calls(f"^::((std::)?(strto[dfl]|strtold|atof|sscanf|scanf|fscanf)|{STD[2:]}sto(f|d|ld))$"),
            f'callExpr(callee(functionDecl(matchesName("^{STD}(to_string|to_chars|from_chars)$"))), '
            "hasAnyArgument(hasType(realFloatingPointType())))",
            'callExpr(callee(functionDecl(matchesName("^::(std::)?(printf|fprintf|sprintf|snprintf)$"))), '
            "hasAnyArgument(hasType(realFloatingPointType())))",
            'cxxOperatorCallExpr(hasAnyOverloadedOperatorName("<<", ">>"), '
            "hasArgument(1, hasType(realFloatingPointType())))",
        ],
        "instead": "reading and writing floats differs between libraries: read quantities as text with units into "
        "whole numbers (A3.6), and show floats only in view/",
    },
    {
        "name": "random",
        "where": SIM,
        "match": [
            type_named(
                f"^{STD}([a-z0-9_]*_distribution|[a-z_]*_engine|random_device|seed_seq|mt19937(_64)?|"
                "minstd_rand0?|ranlux[0-9_a-z]*|knuth_b)$"
            ),
            calls(f"^{STD}(ranges::)?(shuffle|random_shuffle|sample)$"),
            f'declRefExpr(to(varDecl(matchesName("^{STD}ranges::(shuffle|sample)$"))))',
        ],
        "instead": "<random>'s distributions and shuffles differ between libraries: draw with kd::chance",
    },
    {
        "name": "reduce",
        "where": SIM,
        "match": [
            calls(f"^{STD}(transform_)?(reduce|inclusive_scan|exclusive_scan)$"),
            f'declRefExpr(to(varDecl(matchesName("^{STD}execution::"))))',
        ],
        "instead": "these add in any order: add pieces of a fixed size in their order (kd::run::Workers)",
    },
    {
        "name": "hash-unordered",
        "where": SIM,
        "match": [type_named(f"^{STD}(hash|unordered_(map|set|multimap|multiset))$")],
        "instead": "std::hash and the unordered containers' order differ between libraries: use std::map or a "
        "sorted std::vector, and kd::chance::name for a stable hash",
    },
    {
        "name": "sort",
        "where": SIM,
        "match": [
            calls(f"^{STD}(sort|partial_sort|partial_sort_copy|nth_element|make_heap|push_heap|pop_heap|sort_heap)$"),
            f'declRefExpr(to(varDecl(matchesName("^{STD}ranges::(sort|partial_sort|partial_sort_copy|nth_element|'
            'make_heap|push_heap|pop_heap|sort_heap)$"))))',
            type_named(f"^{STD}priority_queue$"),
        ],
        "instead": "ties come out in a different order on each library: use kd::num::sort_strict, which refuses "
        "ties, or std::stable_sort",
        "exempt": {"sim/src/kd/num/sort.hpp": "the strict sort itself"},
    },
    {
        "name": "outside-input",
        "where": SIM,
        "match": [
            calls(r"^::(std::thread::hardware_concurrency|sysconf|get_nprocs|get_nprocs_conf)$"),
            f'callExpr(callee(cxxMethodDecl(hasName("now"), ofClass(matchesName("^{STD}chrono::")))))',
            calls(r"^::(time|clock|gettimeofday|clock_gettime|setlocale|localeconv)$"),
            type_named(f"^{STD}locale$"),
            calls(
                r"^::(std::)?(is(alnum|alpha|blank|cntrl|digit|graph|lower|print|punct|space|upper|xdigit)|"
                r"to(lower|upper))$"
            ),
            'castExpr(hasCastKind("CK_PointerToIntegral"))',
            'binaryOperator(hasAnyOperatorName("<", ">", "<=", ">="), hasLHS(hasType(pointerType())))',
        ],
        "instead": "thread counts, clocks, the locale and addresses differ from run to run: decide only from the "
        "world's state and kd::chance",
    },
    {
        "name": "two-effects",
        "where": SIM,
        "match": [f"callExpr(hasArgument({a}, {EFFECT}), hasArgument({b}, {EFFECT}))" for a, b in PAIRS]
        + [
            f"cxxConstructExpr(unless(isListInitialization()), hasArgument({a}, {EFFECT}), hasArgument({b}, {EFFECT}))"
            for a, b in PAIRS
        ]
        + [
            f"binaryOperator(unless(hasAnyOperatorName({SEQUENCED})), hasLHS({EFFECT}), hasRHS({EFFECT}))",
            f"cxxOperatorCallExpr(unless(hasAnyOverloadedOperatorName({SEQUENCED})), "
            f"hasArgument(0, {EFFECT}), hasArgument(1, {EFFECT}))",
        ],
        "instead": "the order of two calls with effects in one expression differs between compilers: give each its "
        "own statement",
    },
    {
        "name": "raw-memory",
        "where": SIM,
        "match": [
            "cxxReinterpretCastExpr()",
            calls(r"^::(std::)?(memcpy|memmove|memcmp)$"),
            calls(f"^{STD}as_(writable_)?bytes$"),
        ],
        "instead": "raw memory holds padding and the chip's byte order: write fields one by one (kd::num::Digest) "
        "and use std::bit_cast for a value's bits",
        "exempt": {"sim/src/kd/num/digest.cpp": "the digest's own buffer, which holds only the fields written"},
    },
]
LOCATION = re.compile(r'^(.+?):(\d+):(\d+): note: "([a-z-]+)" binds here$')


def relative(path):
    return os.path.relpath(os.path.abspath(path), ROOT).replace(os.sep, "/")


def query_text():
    """The clang-query script: every rule's matchers, each bound to the rule's name."""
    lines = ["set output diag", "set bind-root false"]
    for rule in RULES:
        lines += [f'match {m}.bind("{rule["name"]}")' for m in rule["match"]]
    return "\n".join(lines) + "\n"


def applies(rule, path):
    return path.startswith(rule["where"]) and path not in rule.get("exempt", {})


def problems_in(output):
    """(path, line, rule) for each match in our own files that its rule applies to."""
    by_name = {r["name"]: r for r in RULES}
    found = set()
    for line in output.splitlines():
        m = LOCATION.match(line.strip())
        if not m:
            continue
        path, row, name = relative(m.group(1)), int(m.group(2)), m.group(4)
        if applies(by_name[name], path):
            found.add((path, row, name))
    return found


def run_query(build, path, script):
    done = subprocess.run([QUERY, "-p", build, "-f", script, path], capture_output=True, text=True)
    if done.returncode != 0 or "Error" in done.stderr:
        return None, done.stdout + done.stderr
    return problems_in(done.stdout), ""


def check(build, files, cache=True):
    """(problems, how many files were read) for these files as the build folder compiles them."""
    with open(os.path.join(build, "compile_commands.json")) as f:
        entries = {os.path.realpath(e["file"]): e for e in json.load(f)}
    text = query_text()
    version = subprocess.run([QUERY, "--version"], capture_output=True, text=True).stdout.encode()
    passed = os.path.join(build, "rules-passed")
    os.makedirs(passed, exist_ok=True)
    todo = []
    for path in files:
        entry = entries.get(os.path.realpath(path))
        deps = cppcache.depends(build, entry["output"]) if entry and cache else None
        stamp = None
        if deps is not None:
            parts = [("version", version), ("rules", text.encode()), ("command", entry["command"].encode())]
            stamp = os.path.join(passed, cppcache.fingerprint(parts + [(d, cppcache.contents(d)) for d in deps]))
            if os.path.exists(stamp):
                continue
        todo.append((path, stamp))
    with tempfile.NamedTemporaryFile("w", suffix=".query", delete=False) as f:
        f.write(text)
        script = f.name
    try:

        def one(item):
            path, stamp = item
            found, error = run_query(build, path, script)
            if found is None:
                return [f"{relative(path)}: clang-query failed:\n{error}"]
            if not found and stamp:
                with open(stamp, "w"):
                    pass
            return [f"{p}:{n}: {name}: {rule_of(name)['instead']}" for p, n, name in sorted(found)]

        with concurrent.futures.ThreadPoolExecutor(os.cpu_count() or 1) as pool:
            problems = sorted({p for found in pool.map(one, todo) for p in found})
    finally:
        os.remove(script)
    return problems, len(todo)


def rule_of(name):
    return next(r for r in RULES if r["name"] == name)


def main(argv):
    if argv[:1] == ["list"]:
        for r in RULES:
            print(f"{r['name']} ({', '.join(r['where'])}): {r['instead']}")
            for path, why in r.get("exempt", {}).items():
                print(f"    except {path}: {why}")
        return 0
    if len(argv) < 2 or argv[0] != "check":
        print(__doc__)
        return 2
    problems, read = check(argv[1], argv[2:])
    for p in problems:
        print(f"Rules: {p}")
    if problems:
        print(f"Rules: FAIL ({len(problems)} problems)")
        return 1
    print(f"Rules: {len(RULES)} rules, {read} files read, {len(argv) - 2 - read} unchanged since they passed")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
