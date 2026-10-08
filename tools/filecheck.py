#!/usr/bin/env python3
"""Text checks over the repository (A17).

    python3 tools/filecheck.py file                the file check (PRC-10) and the commit check (PRC-07)
    python3 tools/filecheck.py ids --merge         the coverage check (PRC-12)
    python3 tools/filecheck.py where <ID>          where the code does an item: every line naming it (PRC-12)
    python3 tools/filecheck.py note                dist/NOTE.md's five headings and its APK link (PRC-11)
    python3 tools/filecheck.py selftest            each planted fault fails with its own message; the clean
                                                   fixture passes (PRC-12)

The code is the index: code names the items it implements and tests the items they check, so `where` finds any
item's code, and no document keeps a list of where things are done.

The file check reads PROJECT.md's item markers, statuses, IDs (unique, never retired ones) and generated lists;
every ID and architecture section the three documents cite; the plan's milestones (`## M1 ...`, four fields each,
in order) and steps (`### α1.2b ...`, six fields each, under their own milestone, serving only its items, with
task IDs of their own); and every commit since main that changes PROJECT.md. Documented exceptions sit in
tools/filecheck-known.txt. Python's standard library only.
"""

import difflib
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DOCUMENTS = ("PROJECT.md", "ARCHITECTURE.md", "IMPLEMENTATION.md")
KNOWN = "tools/filecheck-known.txt"
FIXTURES = os.path.join(ROOT, "tools", "tests", "filecheck")

ID = r"[A-Z]{3}-\d{2,3}"
ID_RE = re.compile(rf"\b{ID}\b")
STATUSES = ("Decided", "Proposed", "To test")
MARKER = re.compile(rf"^\s*(?:- |\d+\. )`({ID})` \*\*(.+?)\*\* \*\(([^)]*)\)\*(?::.*)?$")
# Lines that set out to be markers: a list entry opening with a backticked token and a bold name, or with something
# shaped like an ID however it is wrapped, so a marker that lost its backticks or bold is still caught.
MARKER_LIKE = (
    re.compile(r"^\s*(?:-|\d+\.)\s+`([^`]+)`\s+\*\*"),
    re.compile(r"^\s*(?:[-*+]|\d+\.)\s+(?:\*\*)?`?([A-Z]{2,4}-?\d{1,4})\b"),
)
BACKTICKED = re.compile(r"`([^`\n]+)`")
RETIRED_LINE = re.compile(r"^\s*(?:- )?Retired IDs:(.*)$", re.M)
EXAMPLES = "## How this file works"  # its IDs are examples, not references
SECTION = re.compile(r"(?<![\w.#/-])(A\d{1,2}(?:\.\d{1,2})?)(?!\w)")
ARCH_HEADING = re.compile(r"^#{1,6}\s+(A\d+(?:\.\d+)*)\.?\s", re.M)
MILESTONE_HEADING = re.compile(r"^## M(\d{1,2}) \S")
ALPHA_HEADING = re.compile(r"^### α((\d{1,2})\.\d{1,2}[a-e]?) \S")  # α1.2b: milestone 1, alpha 2, step b
MILESTONE_FIELDS = ["Goal", "Serves", "You will see", "Risks"]
ALPHA_FIELDS = ["Goal", "Serves", "Architecture", "Tasks", "Tests", "On the phone"]
FIELD = re.compile(r"^\*\*([A-Z][^*:]*?):\*\*")
TASK = re.compile(r"^\d+\. `([^`]+)`")
TASK_ID = re.compile(r"T\d{1,2}\.\d{1,2}[a-e]?\.\d+")
GENERATED = re.compile(r"<!-- generated: ([a-z ]+) -->\n(.*?)<!-- end generated -->", re.S)
NOTE_HEADINGS = ["What is new", "What to try", "What is rough", "IDs delivered", "Links"]
RULES_HEADING = "## Rules every alpha keeps"


# PROJECT.md (PRC-10's file check).


class Item:
    """One PROJECT.md item: ID, name, status, and its lines (0-based, from its marker to the line before the next
    marker or heading)."""

    def __init__(self, id_, name, status, line):
        self.id, self.name, self.status, self.start, self.end = id_, name, status, line, line


def without_examples(text):
    """PROJECT.md's lines with `## How this file works` blanked, since its IDs are examples."""
    out, skip = [], False
    for line in text.split("\n"):
        if line.startswith("## "):
            skip = line.strip() == EXAMPLES
        out.append("" if skip else line)
    return out


def retired(project):
    m = RETIRED_LINE.search(project)
    return set(ID_RE.findall(m.group(1))) if m else set()


def items_of(project):
    """PROJECT.md's items by ID in file order, and the problems with their markers.
    Implements PRC-10, see A17."""
    gone = retired(project)
    items, problems, current = {}, [], None
    for i, line in enumerate(without_examples(project)):
        if line.startswith("#"):
            current = None
            continue
        like = next((m for m in (p.match(line) for p in MARKER_LIKE) if m), None)
        if not like:
            if current:
                current.end = i
            continue
        m = MARKER.match(line)
        if not m:
            token = like.group(1)
            what = (
                "is not an ID (three capitals, a dash, 2 or 3 digits)"
                if not re.fullmatch(ID, token)
                else "has a marker that does not read as `ID` **Name** *(Status)*"
            )
            problems.append(f"PROJECT.md line {i + 1}: `{token}` {what}")
            current = None
            continue
        id_, name, status = m.groups()
        if status not in STATUSES:
            problems.append(
                f"PROJECT.md line {i + 1}: `{id_}` has the status *{status}*, not one of {', '.join(STATUSES)}"
            )
        if id_ in gone:
            problems.append(f"PROJECT.md line {i + 1}: `{id_}` is retired and may not be used again")
            current = None
            continue
        if id_ in items:
            problems.append(f"PROJECT.md line {i + 1}: `{id_}` is defined twice (first on line {items[id_].start + 1})")
            current = None
            continue
        current = items[id_] = Item(id_, name, status, i)
    return items, problems


def citations(text, areas):
    """(line from 1, ID) for each ID a text cites: anything shaped like an ID inside backticks, and outside them only
    IDs of a known area, so `SHA-256` in prose is never taken for one."""
    out = []
    for n, line in enumerate(text.split("\n"), 1):
        for span in BACKTICKED.findall(line):
            out += [(n, x) for x in ID_RE.findall(span)]
        out += [(n, x) for x in ID_RE.findall(BACKTICKED.sub(" ", line)) if x[:3] in areas]
    return out


def anchor(heading):
    """A heading's link anchor as the Markdown viewers make it."""
    return re.sub(r"[^\w\- ]", "", heading.lower()).replace(" ", "-")


def generated_lists(project, items):
    """What each generated list of PROJECT.md must hold: the contents exactly; the open items and the proposals by
    their IDs, in file order (their words are the writer's)."""
    lines = []
    for h in re.findall(r"^## (.+)$", project, re.M):
        if h == "Contents":
            continue
        m = re.fullmatch(r"(\d+)\. (.+)", h)
        # One sentence a line, as the whole file is written: "1." ends a line of its own.
        text = f"{m.group(1)}.\n  {m.group(2)}" if m else h
        lines.append(f"- [{text}](#{anchor(h)})")
    return {
        "contents": "\n".join(lines) + "\n",
        "open items": [i.id for i in items.values() if i.status == "To test"],
        # A proposal is a Proposed item, or a decided one with a **Proposed change:** beneath it (PRC-07).
        "proposals": [
            i.id for i in items.values() if i.status == "Proposed" or proposal_lines(without_examples(project), i)
        ],
    }


def check_project(project):
    """The file check on PROJECT.md: its markers, citations and generated lists. Returns (items, problems, count)."""
    items, problems = items_of(project)
    gone = retired(project)
    areas = {i[:3] for i in items}
    # A marker's own ID defines its item rather than citing it, even when the marker is malformed.
    lines = [
        BACKTICKED.sub(" ", line, count=1) if any(p.match(line) for p in MARKER_LIKE) else line
        for line in without_examples(project)
    ]
    cited = citations("\n".join(lines), areas)
    for n, x in cited:
        if x in gone and x not in items:
            problems.append(f"PROJECT.md line {n}: cites the retired `{x}`")
        elif x not in items:
            problems.append(f"PROJECT.md line {n}: `{x}` is not defined")
    want = generated_lists(project, items)
    found = {name: body for name, body in GENERATED.findall(project)}
    for name in ("contents", "open items", "proposals"):
        if name not in found:
            problems.append(f"PROJECT.md: no generated list '{name}'")
            continue
        body = found[name]
        if name == "contents":
            ok = body == want[name]
        else:
            listed = ID_RE.findall(body)
            ok = listed == want[name] and (listed or body.strip() == "- None at present.")
        if not ok:
            should = "" if name == "contents" else f" (it should list {', '.join(want[name]) or 'none'})"
            problems.append(f"PROJECT.md: the generated list '{name}' is not current{should}")
    return items, problems, len(cited)


# ARCHITECTURE.md and IMPLEMENTATION.md.


def check_citations(name, text, items, gone, sections):
    """Every ID a document cites is a live item, and every section it cites is a heading of ARCHITECTURE.md."""
    problems, count = [], 0
    areas = {i[:3] for i in items} | {i[:3] for i in gone}
    for n, x in citations(text, areas):
        count += 1
        if x in gone:
            problems.append(f"{name} line {n}: cites the retired `{x}`")
        elif x not in items:
            problems.append(f"{name} line {n}: `{x}` is not defined in PROJECT.md")
    for n, line in enumerate(text.split("\n"), 1):
        if name == "ARCHITECTURE.md" and line.startswith("#"):
            continue  # a heading defines its section
        for s in SECTION.findall(line):
            count += 1
            if s not in sections:
                problems.append(f"{name} line {n}: section {s} is not a heading of ARCHITECTURE.md")
    return problems, count


def milestone_sections(plan):
    """(milestone number, heading line from 1, its own lines up to its first step) for each `## M<n>` section."""
    lines = plan.split("\n")
    out = []
    for i, line in enumerate(lines):
        m = MILESTONE_HEADING.match(line)
        if m:
            j = i + 1
            while j < len(lines) and not re.match(r"^#{1,3} ", lines[j]):
                j += 1
            out.append((int(m.group(1)), i + 1, lines[i + 1 : j]))
    return out


def alpha_sections(plan):
    """(step such as `1.2b`, the milestone its name gives, the milestone it sits under or None, heading line from 1,
    the section's lines) for each `### α..` section of the plan."""
    lines = plan.split("\n")
    out, under = [], None
    for i, line in enumerate(lines):
        if line.startswith("## "):
            m = MILESTONE_HEADING.match(line)
            under = int(m.group(1)) if m else None
        a = ALPHA_HEADING.match(line)
        if a:
            j = i + 1
            while j < len(lines) and not re.match(r"^#{1,3} ", lines[j]):
                j += 1
            out.append((a.group(1), int(a.group(2)), under, i + 1, lines[i + 1 : j]))
    return out


def field(lines, label):
    """The text of a milestone's or step's field: from its label to the next label."""
    out, inside = [], False
    for line in lines:
        f = FIELD.match(line)
        if f:
            inside = f.group(1) == label
        if inside:
            out.append(line)
    return "\n".join(out)


def labels(lines, wanted):
    return [f.group(1) for f in map(FIELD.match, lines) if f and f.group(1) in wanted]


def check_plan(plan):
    """Each milestone holds its four fields in order, and is numbered one after the one before it; each step holds
    its six fields in order, sits under its own milestone, serves only items its milestone's Serves line names, and
    its task lines carry unique IDs of its own."""
    problems, seen, named, before = [], {}, {}, None
    for n, line, own in milestone_sections(plan):
        found = labels(own, MILESTONE_FIELDS)
        if found != MILESTONE_FIELDS:
            problems.append(f"IMPLEMENTATION.md line {line}: M{n}'s fields are {found}, not the four in order")
        if before is not None and n != before + 1:
            problems.append(f"IMPLEMENTATION.md line {line}: M{n} follows M{before}")
        before = n
        named[n] = set(ID_RE.findall(field(own, "Serves")))
    for alpha, number, under, line, body in alpha_sections(plan):
        found = labels(body, ALPHA_FIELDS)
        if found != ALPHA_FIELDS:
            problems.append(f"IMPLEMENTATION.md line {line}: α{alpha}'s fields are {found}, not the six in order")
        if under != number:
            where = f"M{under}" if under is not None else "no milestone"
            problems.append(f"IMPLEMENTATION.md line {line}: α{alpha} sits under {where}, not M{number}")
        else:
            for x in ID_RE.findall(field(body, "Serves")):
                if x not in named[under]:
                    problems.append(
                        f"IMPLEMENTATION.md line {line}: α{alpha} serves `{x}`, which M{under}'s Serves "
                        "line doesn't name"
                    )
        in_tasks = False
        for k, text in enumerate(body, line + 1):
            f = FIELD.match(text)
            if f:
                in_tasks = f.group(1) == "Tasks"
            t = TASK.match(text)
            if in_tasks and re.match(r"^\d+\.\s", text) and not t:
                problems.append(f"IMPLEMENTATION.md line {k}: a numbered line of α{alpha}'s tasks has no task ID")
            if not t:
                continue
            task = t.group(1)
            if not re.fullmatch(rf"T{re.escape(alpha)}\.\d+", task):
                problems.append(f"IMPLEMENTATION.md line {k}: task `{task}` is not one of α{alpha}'s (T{alpha}.<n>)")
            if task in seen:
                problems.append(f"IMPLEMENTATION.md line {k}: task `{task}` repeats line {seen[task]}")
            seen.setdefault(task, k)
    return problems


# The commit check (PRC-07).


def changed_ids(old, new):
    """The IDs whose item lines differ between two versions of PROJECT.md."""
    old_items, _ = items_of(old)
    new_items, _ = items_of(new)

    def owner(items, line):
        return next((i.id for i in items.values() if i.start <= line <= i.end), None)

    ids = set()
    a, b = old.split("\n"), new.split("\n")
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, a, b, autojunk=False).get_opcodes():
        if tag != "equal":
            ids.update(owner(old_items, i) for i in range(i1, i2))
            ids.update(owner(new_items, j) for j in range(j1, j2))
    ids.discard(None)
    return ids


CHANGED = re.compile(rf"^Changed:\s*(?P<ids>{ID}(?:,\s*{ID})*)\s*\((?P<why>.*)\)$", re.S)


def changed_entries(message):
    """A commit message's `Changed:` entries, each with the lines its brackets run onto, joined by spaces."""
    entries, current = [], None
    for line in message.split("\n"):
        if line.strip().startswith("Changed:"):
            current = [line.strip()]
            entries.append(current)
        elif current is not None and " ".join(current).count("(") > " ".join(current).count(")"):
            current.append(line.strip())
        else:
            current = None
    return [" ".join(e) for e in entries]


PROPOSED_CHANGE = "**Proposed change:**"


def proposal_lines(lines, item):
    """The lines of an item's **Proposed change:** entries: each from its own line through the lines indented deeper
    than it. They wait for the owner's OK (PRC-07), so they change none of a decided item's words."""
    out, depth = set(), None
    for i in range(item.start, min(item.end, len(lines) - 1) + 1):
        line = lines[i]
        indent = len(line) - len(line.lstrip(" "))
        if PROPOSED_CHANGE in line:
            out.add(i)
            depth = indent
        elif depth is not None and line.strip() and indent > depth:
            out.add(i)
        else:
            depth = None
    return out


def item_words(project, item):
    """An item's text with its spacing evened and its proposed changes left out, so a blank line, a rewrapped
    sentence or a proposal changes none of its words."""
    lines = project.split("\n")
    skip = proposal_lines(lines, item)
    return " ".join(" ".join(lines[i] for i in range(item.start, item.end + 1) if i not in skip).split())


def needs_owner(old, new):
    """The IDs whose words a change makes or changes while Decided, or takes from Decided (PRC-07)."""
    old_items, _ = items_of(old)
    new_items, _ = items_of(new)
    out = set()
    for id_ in set(old_items) | set(new_items):
        a, b = old_items.get(id_), new_items.get(id_)
        if "Decided" not in (a and a.status, b and b.status):
            continue
        if a is None or b is None or item_words(old, a) != item_words(new, b):
            out.add(id_)
    return out


def check_commit(sha, message, old, new):
    """A commit changing PROJECT.md names every ID whose lines it changed on its `Changed:` lines, each with its
    reason in brackets, and the owner's OK for every item it makes Decided or changes while Decided (PRC-07); a
    **Proposed change:** beneath a decided item needs only its reason.
    Implements PRC-07, see A17."""
    entries = changed_entries(message)
    if not entries:
        return [f"commit {sha[:12]} changes PROJECT.md with no `Changed:` line (PRC-07)"]
    problems, named, approved, unreasoned = [], set(), set(), set()
    for e in entries:
        ids = set(ID_RE.findall(e))
        named |= ids
        m = CHANGED.match(e)
        if not m or not ID_RE.sub("", m.group("why")).strip(" ,;:"):
            unreasoned |= ids
            problems.append(f"commit {sha[:12]}: `{e[:60]}` gives no reason in brackets after its IDs (PRC-07)")
        elif "owner OK" in m.group("why"):
            approved |= set(ID_RE.findall(m.group("ids")))
    missing = sorted(changed_ids(old, new) - named)
    if missing:
        problems.append(
            f"commit {sha[:12]} changes {', '.join(missing)} in PROJECT.md without naming them on `Changed:`"
        )
    # An ID left unnamed, or named without a reason, has failed already.
    unapproved = sorted((needs_owner(old, new) & named) - approved - unreasoned)
    if unapproved:
        problems.append(
            f"commit {sha[:12]} makes or changes {', '.join(unapproved)} as Decided with no `owner OK` "
            "in its reason (PRC-07)"
        )
    return problems


def file_check(project, arch, plan, commits):
    """The whole file check: (problems, item count, citation count). `commits` holds (sha, message, PROJECT.md
    before, PROJECT.md after) for each commit to check."""
    items, problems, count = check_project(project)
    gone = retired(project)
    sections = set(ARCH_HEADING.findall(arch))
    for name, text in (("ARCHITECTURE.md", arch), ("IMPLEMENTATION.md", plan)):
        p, c = check_citations(name, text, items, gone, sections)
        problems += p
        count += c
    problems += check_plan(plan)
    for sha, message, old, new in commits:
        problems += check_commit(sha, message, old, new)
    return problems, len(items), count


# The coverage check (PRC-12, A17).

# Where the code that names items lives, and how each language names them (the plan's Conventions): C++, shaders and
# the Android plug-in with `///` and `// checks:`; GDScript with `##` and `# checks:`; Python and shell with
# `# checks:`; catalogues and scenes in data/ with `checks = [...]`. Only the game's layers and the tools are read;
# third-party code and builds are skipped.
SLASH = (".cpp", ".cc", ".h", ".hpp", ".gdshader", ".gdshaderinc", ".glsl", ".kt", ".java")
GDSCRIPT = (".gd",)
HASH = (".py", ".sh")
SCANNED = ("game", "view", "sim", "data", "android", "tools")
SKIPPED = {"build", ".godot", "addons", "thirdparty", "godot-cpp", ".gradle", "__pycache__"}
CHECKS_SETTING = re.compile(r"^\s*checks\s*=\s*\[([^\]]*)\]")
# How each kind of test begins, the line among the four above it that must name what it checks, and its name in
# messages: a doctest TEST_CASE, a gdUnit4 test (in a GdUnitTestSuite), a Python unittest.
TESTS = {
    "cpp": (
        re.compile(r"^\s*(?:TEST_CASE|TEST_CASE_FIXTURE|TEST_CASE_TEMPLATE|SCENARIO)\s*\("),
        "// checks:",
        "a doctest TEST_CASE",
    ),
    "gd": (re.compile(r"^func test_"), "# checks:", "a gdUnit4 test"),
    "py": (re.compile(r"^\s*def test_"), "# checks:", "a Python test"),
}


def kinds_of(project, items):
    """Each item's kind (Feature, Rule or Context) from `### Kinds of item`: the area defaults and the exceptions;
    an area no line names holds features."""
    areas, exceptions = {}, {}
    m = re.search(r"^### Kinds of item\n(.*?)(?=^#)", project, re.M | re.S)
    for line in (m.group(1) if m else "").split("\n"):
        k = re.match(r"^- \*\*(Context|Rules|Features):\*\*", line)
        if k:
            kind = {"Context": "Context", "Rules": "Rule", "Features": "Feature"}[k.group(1)]
            for token in BACKTICKED.findall(line):
                if re.fullmatch(ID, token):
                    exceptions[token] = kind
                elif re.fullmatch(r"[A-Z]{3}", token):
                    areas[token] = kind
    return {i: exceptions.get(i, areas.get(i[:3], "Feature")) for i in items}


def id_lines(path, text):
    """(line from 1, kind, IDs) for each line naming IDs: `// checks:` and `# checks:` lines, `Implements` doc
    lines (`///` in C++ and shaders, `##` in GDScript), and a catalogue entry's or scene's `checks = [..]`."""
    out = []
    slash, gd, hashed = path.endswith(SLASH), path.endswith(GDSCRIPT), path.endswith(HASH)
    setting = path.startswith("data/") and path.endswith(".toml")
    for n, line in enumerate(text.split("\n"), 1):
        s = line.strip()
        if (slash and s.startswith("// checks:")) or ((gd or hashed) and s.startswith("# checks:")):
            out.append((n, "checks", ID_RE.findall(s)))
        elif slash and s.startswith(("///", "//!")) and "mplements" in s:
            out.append((n, "implements", ID_RE.findall(s)))
        elif gd and s.startswith("##") and "mplements" in s:
            out.append((n, "implements", ID_RE.findall(s)))
        elif setting and CHECKS_SETTING.match(line):
            out.append((n, "checks", ID_RE.findall(CHECKS_SETTING.match(line).group(1))))
    return out


def test_kind(path, text):
    """Which kind of test a file holds ("cpp", "gd" or "py"), or None."""
    if path.endswith((".cpp", ".cc", ".h", ".hpp")):
        return "cpp"
    if path.endswith(GDSCRIPT) and re.search(r"^extends\s+GdUnitTestSuite\b", text, re.M):
        return "gd"
    if path.endswith(".py") and os.path.basename(path).startswith("test_"):
        return "py"
    return None


def unnamed_tests(path, text):
    """(count of tests, problems) for a file: each test needs a checks line naming an ID among the four lines above
    it."""
    kind = test_kind(path, text)
    if kind is None:
        return 0, []
    start, marker, what = TESTS[kind]
    lines, count, problems = text.split("\n"), 0, []
    for i, line in enumerate(lines):
        if start.match(line):
            count += 1
            above = [x.strip() for x in lines[max(0, i - 4) : i]]
            if not any(x.startswith(marker) and ID_RE.search(x) for x in above):
                problems.append(f"{path}:{i + 1}: {what} with no `{marker}` line in the four lines above")
    return count, problems


def section_of(plan, heading):
    m = re.search(r"^" + re.escape(heading) + r"\n(.*?)(?=^## |\Z)", plan, re.M | re.S)
    return m.group(1) if m else ""


def tasks_of(plan):
    """(task ID, line from 1, text) for each task: its numbered line up to the next task, field or heading."""
    out, current = [], None
    for n, line in enumerate(plan.split("\n"), 1):
        t = TASK.match(line)
        if t and TASK_ID.fullmatch(t.group(1)):
            current = [t.group(1), n, [line]]
            out.append(current)
        elif current and (FIELD.match(line) or line.startswith("#")):
            current = None
        elif current:
            current[2].append(line)
    return [(t, n, "\n".join(lines)) for t, n, lines in out]


def serves(plan):
    """{ID: [milestones and steps]} from every milestone's and step's Serves line, in plan order."""
    out = {}
    entries = [(f"M{n}", own) for n, _, own in milestone_sections(plan)]
    entries += [(f"α{alpha}", body) for alpha, _, _, _, body in alpha_sections(plan)]
    for name, lines in entries:
        for x in ID_RE.findall(field(lines, "Serves")):
            out.setdefault(x, [])
            if name not in out[x]:
                out[x].append(name)
    return out


def coverage(files):
    """The coverage check over a repository given as {path: text}: (problems, a summary).
    Implements PRC-12, see A17."""
    project, plan = files["PROJECT.md"], files["IMPLEMENTATION.md"]
    items, _ = items_of(project)
    gone, kinds = retired(project), kinds_of(project, items)
    problems, tested, implemented, named, tests = [], set(), set(), 0, {"cpp": 0, "gd": 0, "py": 0}
    for path in sorted(p for p in files if p not in DOCUMENTS):
        text = files[path]
        # Every ID named in code, tests, catalogues and scenes is a live item.
        for n, kind, ids in id_lines(path, text):
            if kind == "checks" and not ids:
                problems.append(f"{path}:{n}: a checks line names no ID")
            for x in ids:
                named += 1
                if x in gone:
                    problems.append(f"{path}:{n}: names the retired `{x}`")
                elif x not in items:
                    problems.append(f"{path}:{n}: `{x}` is not defined in PROJECT.md")
                else:
                    (tested if kind == "checks" else implemented).add(x)
        # Every test names what it checks on a checks line among the four lines above it.
        count, unnamed = unnamed_tests(path, text)
        if count:
            tests[test_kind(path, text)] += count
        problems += unnamed
    # Every task names an item.
    tasks = tasks_of(plan)
    for t, n, text in tasks:
        if not any(x in items for x in ID_RE.findall(text)):
            problems.append(f"IMPLEMENTATION.md line {n}: task {t} names no item of PROJECT.md")
    # Every live feature and rule is served by a milestone or step still in the plan, kept by every alpha, or built
    # (named by an `Implements` line), since the plan keeps no record of done work.
    kept = set(ID_RE.findall(section_of(plan, RULES_HEADING)))
    served = serves(plan)
    live = [i for i in items.values() if i.status != "Proposed" and kinds[i.id] in ("Feature", "Rule")]
    for it in live:
        if it.id not in served and it.id not in kept and it.id not in implemented:
            problems.append(
                f"`{it.id}` ({kinds[it.id].lower()}) is not mapped: no milestone's or step's Serves line "
                f"names it, nor {RULES_HEADING}, nor an Implements line"
            )
    # Every ID code implements, the kept rules aside, is named by a test, a scene or a catalogue entry.
    for x in sorted(implemented - kept - tested):
        problems.append(f"`{x}` is implemented in code, but no test, scene or catalogue entry names it")
    summary = (
        f"{named} IDs named in code and tests; {tests['cpp']} C++, {tests['gd']} gdUnit4 and {tests['py']} Python "
        f"tests; {len(tasks)} tasks, "
        f"{len(live)} features and rules mapped, {len(implemented)} IDs implemented in code"
    )
    return problems, summary


def repo_files(root=ROOT):
    """{path: text} for PROJECT.md, IMPLEMENTATION.md and every source the coverage check reads."""
    files = {p: read(os.path.join(root, p)) for p in ("PROJECT.md", "IMPLEMENTATION.md")}
    for top in SCANNED:
        for dirpath, dirnames, filenames in os.walk(os.path.join(root, top)):
            rel = os.path.relpath(dirpath, root).replace(os.sep, "/")
            dirnames[:] = sorted(d for d in dirnames if d not in SKIPPED and f"{rel}/{d}" != "tools/tests/filecheck")
            for name in sorted(filenames):
                path = f"{rel}/{name}"
                if name.endswith(SLASH + GDSCRIPT + HASH) or (top == "data" and name.endswith(".toml")):
                    files[path] = read(os.path.join(root, path))
    return files


# The note (PRC-11).


def check_note(text):
    """Problems with an alpha's note: its five headings, and a link to the APK."""
    headings = {m.strip() for m in re.findall(r"^#{1,6}\s+(.+)$", text, re.M)}
    problems = [f"no heading '{h}'" for h in NOTE_HEADINGS if h not in headings]
    if not re.search(r"https?://\S+/dist/kindling\.apk\b", text):
        problems.append("no link to dist/kindling.apk")
    return problems


# The self-test: one planted fault per message, each failing with it alone, and the clean fixture passing.


def fixture(case_dir):
    out = {}
    for dirpath, _, names in os.walk(case_dir):
        for name in names:
            full = os.path.join(dirpath, name)
            out[os.path.relpath(full, case_dir).replace(os.sep, "/")] = read(full)
    return out


def edit(text, old, new):
    assert old in text, f"planted fault: {old!r} not in the fixture"
    return text.replace(old, new, 1)


# (name, the fault: {path: (old, new)}, or a "commit" message, the message it must give alone).
EXTRA = "- `CTX-01` **Background** *(Decided)*: Context only."
PLANTED = [
    ("not an ID", {"PROJECT.md": (EXTRA, EXTRA + "\n- `CTX1` **Extra** *(Decided)*: x.")}, "is not an ID"),
    ("marker", {"PROJECT.md": (EXTRA, EXTRA + "\n- `CTX-05` Extra *(Decided)*: x.")}, "does not read as"),
    ("status", {"PROJECT.md": (EXTRA, EXTRA + "\n- `CTX-05` **Extra** *(Maybe)*: x.")}, "has the status *Maybe*"),
    (
        "retired reused",
        {"PROJECT.md": (EXTRA, EXTRA + "\n- `CTX-09` **Extra** *(Decided)*: x.")},
        "may not be used again",
    ),
    ("twice", {"PROJECT.md": (EXTRA, EXTRA + "\n- `CTX-01` **Again** *(Decided)*: x.")}, "is defined twice"),
    ("undefined in PROJECT.md", {"PROJECT.md": ("follows `ONE-01`.", "follows `ONE-07`.")}, "`ONE-07` is not defined"),
    ("retired cited", {"ARCHITECTURE.md": ("Serves `ONE-01`.", "Serves `ONE-09`.")}, "cites the retired `ONE-09`"),
    (
        "section",
        {"IMPLEMENTATION.md": ("**Architecture:** `A1.1`.", "**Architecture:** `A1.4`.")},
        "section A1.4 is not a heading",
    ),
    ("step fields", {"IMPLEMENTATION.md": ("**Tests:** the fixture's tests.\n", "")}, "not the six in order"),
    ("milestone fields", {"IMPLEMENTATION.md": ("**You will see:** more.\n", "")}, "not the four in order"),
    ("milestone order", {"IMPLEMENTATION.md": ("## M2 Two", "## M3 Two")}, "M3 follows M1"),
    (
        "step under another milestone",
        {
            "IMPLEMENTATION.md": [
                ("### α1.1a First", "### α2.1a First"),
                ("1. `T1.1a.1`", "1. `T2.1a.1`"),
                ("2. `T1.1a.2`", "2. `T2.1a.2`"),
            ]
        },
        "α2.1a sits under M1, not M2",
    ),
    (
        "step serves beyond its milestone",
        {"IMPLEMENTATION.md": ("**Serves:** `ONE-01`, `ONE-02`.", "**Serves:** `ONE-01`.")},
        "α1.1a serves `ONE-02`, which M1's Serves line doesn't name",
    ),
    ("task step", {"IMPLEMENTATION.md": ("1. `T1.1a.1`", "1. `T1.1b.1`")}, "is not one of α1.1a's"),
    ("task twice", {"IMPLEMENTATION.md": ("2. `T1.1a.2`", "2. `T1.1a.1`")}, "repeats line"),
    ("task without ID", {"IMPLEMENTATION.md": ("2. `T1.1a.2` **Second", "2. **Second")}, "has no task ID"),
    ("contents", {"PROJECT.md": ("- [1.\n  One](#1-one)", "- [1. One](#1-one)")}, "'contents' is not current"),
    (
        "open items",
        {"PROJECT.md": ("- **Third** (`ONE-03`): measured later.", "- None at present.")},
        "'open items' is not current",
    ),
    ("proposals", {"PROJECT.md": ("**Second** *(Decided)*", "**Second** *(Proposed)*")}, "'proposals' is not current"),
    (
        "no Changed line",
        {"PROJECT.md": ("Its words.", "Its new words."), "commit": "Edit\n"},
        "with no `Changed:` line",
    ),
    (
        "Changed misses one",
        {"PROJECT.md": ("Its words.", "Its new words."), "commit": "Edit\n\nChanged: ONE-01 (x)\n"},
        "changes ONE-02 in PROJECT.md without naming",
    ),
    (
        "Changed without reason",
        {"PROJECT.md": ("Its words.", "Its new words."), "commit": "Edit\n\nChanged: ONE-02\n"},
        "gives no reason in brackets",
    ),
    (
        "Decided without OK",
        {"PROJECT.md": ("Its words.", "Its new words."), "commit": "Edit\n\nChanged: ONE-02 (new words)\n"},
        "with no `owner OK`",
    ),
    (
        "proposal unlisted",
        {"PROJECT.md": ("Its words.", "Its words.\n  - **Proposed change:** New words.\n    Why: x.")},
        "it should list ONE-02",
    ),
    (
        "C++ names undefined",
        {"sim/tests/one_test.cpp": ("// checks: ONE-01", "// checks: ONE-08")},
        "one_test.cpp:3: `ONE-08` is not defined in PROJECT.md",
    ),
    (
        "GDScript names undefined",
        {"game/one.gd": ("## Implements ONE-02", "## Implements ONE-08")},
        "one.gd:1: `ONE-08` is not defined in PROJECT.md",
    ),
    (
        "data names undefined",
        {"data/one.toml": ('checks = ["ONE-01"]', 'checks = ["ONE-08"]')},
        "one.toml:2: `ONE-08` is not defined in PROJECT.md",
    ),
    (
        "code names retired",
        {"sim/tests/one_test.cpp": ("// checks: ONE-01", "// checks: ONE-01 ONE-09")},
        "names the retired `ONE-09`",
    ),
    ("empty checks", {"data/one.toml": ('checks = ["ONE-01"]', "checks = []")}, "a checks line names no ID"),
    (
        "C++ test without checks",
        {"sim/tests/one_test.cpp": ("// checks: ONE-01\n", "")},
        "a doctest TEST_CASE with no `// checks:` line",
    ),
    (
        "gdUnit4 test without checks",
        {"game/test/one_test.gd": ("# checks: ONE-02\n", "")},
        "a gdUnit4 test with no `# checks:` line",
    ),
    (
        "Python test without checks",
        {"tools/tests/test_one.py": ("# checks: ONE-01 ONE-02\n", "")},
        "a Python test with no `# checks:` line",
    ),
    ("task without item", {"IMPLEMENTATION.md": ("(`ONE-02`, A1.1)", "(A1.1)")}, "task T1.1a.2 names no item"),
    (
        "not mapped",
        {"IMPLEMENTATION.md": ("**Serves:** `ONE-03`.", "**Serves:** none.")},
        "`ONE-03` (feature) is not mapped",
    ),
    (
        "implemented untested",
        {"sim/src/one.cpp": ("/// Implements ONE-01", "/// Implements ONE-01 ONE-03")},
        "`ONE-03` is implemented in code, but no test",
    ),
]


# (name, a change that must pass: {path: (old, new) or a list of them, "commit": its message}).
PROPOSAL = "Its words.\n  - **Proposed change:** New words.\n    Why: x."
ACCEPTED = [
    (
        "a proposal needs a reason, not the owner's OK",
        {
            "PROJECT.md": [
                ("Its words.", PROPOSAL),
                (
                    "<!-- generated: proposals -->\n- None at present.",
                    "<!-- generated: proposals -->\n- **Second** (`ONE-02`): new words.",
                ),
            ],
            "commit": "Propose\n\nChanged: ONE-02 (a proposed change: new words)\n",
        },
    ),
]


def run_case(clean, faults):
    files = dict(clean)
    commits = []
    for path, change in faults.items():
        if path != "commit":
            for old, new in change if isinstance(change, list) else [change]:
                files[path] = edit(files[path], old, new)
    if "commit" in faults:
        commits = [("0" * 40, faults["commit"], clean["PROJECT.md"], files["PROJECT.md"])]
    problems, _, _ = file_check(files["PROJECT.md"], files["ARCHITECTURE.md"], files["IMPLEMENTATION.md"], commits)
    return problems + coverage(files)[0]


def selftest():
    """Prints a line a case and the verdict; returns the exit code.
    Implements PRC-12, see A17."""
    clean = fixture(os.path.join(FIXTURES, "clean"))
    bad = run_case(clean, {})
    print(("ok   " if not bad else "FAIL ") + "the clean fixture passes" + "".join(f"\n       {p}" for p in bad))
    failures = bool(bad)
    for name, faults, expect in PLANTED:
        problems = run_case(clean, faults)
        good = len(problems) == 1 and expect in problems[0]
        failures += not good
        print(
            ("ok   " if good else "FAIL ") + f"{name}: " + ("; ".join(problems) or f"passed, but must fail: {expect}")
        )
    for name, change in ACCEPTED:
        problems = run_case(clean, change)
        failures += bool(problems)
        print(("ok   " if not problems else "FAIL ") + f"{name}: " + ("; ".join(problems) or "passes"))
    print(
        f"Selftest: OK ({len(PLANTED)} planted faults each fail alone; the clean fixture and {len(ACCEPTED)} "
        "accepted change pass)"
        if not failures
        else f"Selftest: FAIL ({failures})"
    )
    return 0 if not failures else 1


# Running.


def read(path):
    with open(path, encoding="utf-8") as f:
        return f.read()


def git(*args):
    return subprocess.run(["git", "-C", ROOT, *args], check=True, capture_output=True, text=True).stdout


def show(rev, path):
    r = subprocess.run(["git", "-C", ROOT, "show", f"{rev}:{path}"], capture_output=True, text=True)
    return r.stdout if r.returncode == 0 else ""


def commits_changing(path):
    """(sha, message, before, after) for each commit since main, merges aside, that changes a file."""
    shas = git("log", "--no-merges", "--format=%H", "origin/main..HEAD", "--", path).split()
    return [(s, git("log", "-1", "--format=%B", s), show(f"{s}^", path), show(s, path)) for s in shas]


def known_findings():
    """Documented exceptions: `<finding> | <date> | reason` a line, `#` a comment."""
    path = os.path.join(ROOT, KNOWN)
    if not os.path.exists(path):
        return set()
    lines = (line.strip() for line in read(path).split("\n"))
    return {line.split(" | ")[0].strip() for line in lines if line and not line.startswith("#")}


def report(prefix, problems, ok_line):
    known = known_findings()
    left = [p for p in problems if p not in known]
    for p in sorted(known & set(problems)):
        print(f"{prefix}: documented exception: {p}")
    for p in left:
        print(f"{prefix}: {p}")
    print(ok_line if not left else f"{prefix}: FAIL ({len(left)} problems)")
    return 0 if not left else 1


def where(item, files=None):
    """Prints the item's name and every line of code, tests and data that names it, those implementing it first,
    then those checking it, then any other mention. Implements PRC-12."""
    files = files if files is not None else repo_files()
    items, _ = items_of(files["PROJECT.md"])
    if item not in items:
        print(f"Where: `{item}` is not an item of PROJECT.md")
        return 1
    found = {"implements": [], "checks": [], "mentions": []}
    for path in sorted(p for p in files if p not in DOCUMENTS):
        lines = files[path].split("\n")
        tagged = set()
        for n, kind, ids in id_lines(path, files[path]):
            if item in ids:
                found[kind].append(f"{path}:{n}: {lines[n - 1].strip()}")
                tagged.add(n)
        for n, line in enumerate(lines, 1):
            if n not in tagged and re.search(rf"\b{re.escape(item)}\b", line):
                found["mentions"].append(f"{path}:{n}: {line.strip()}")
    print(f"{item} {items[item].name} ({items[item].status})")
    for kind in ("implements", "checks", "mentions"):
        print(f"  {kind}: {len(found[kind]) or 'none'}")
        for f in found[kind]:
            print(f"    {f}")
    return 0


def main(argv):
    if argv == ["file"]:
        try:
            git("rev-parse", "--verify", "-q", "origin/main")
        except subprocess.CalledProcessError:
            print("File check: fetch main first: git fetch origin +refs/heads/main:refs/remotes/origin/main")
            return 1
        commits = commits_changing("PROJECT.md")
        docs = [read(os.path.join(ROOT, d)) for d in DOCUMENTS]
        problems, n, m = file_check(*docs, commits)
        return report(
            "File check",
            problems,
            f"File check: OK ({n} items, {m} citations, {len(commits)} commits changing PROJECT.md)",
        )
    if argv == ["ids", "--merge"]:
        problems, summary = coverage(repo_files())
        return report("Coverage", problems, f"Coverage: OK ({summary})")
    if argv == ["note"]:
        path = os.path.join(ROOT, "dist", "NOTE.md")
        problems = check_note(read(path)) if os.path.exists(path) else ["dist/NOTE.md is missing"]
        for p in problems:
            print(f"Note: {p}")
        print("Note: OK" if not problems else "Note: FAIL")
        return 0 if not problems else 1
    if argv == ["selftest"]:
        return selftest()
    if len(argv) == 2 and argv[0] == "where":
        return where(argv[1])
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
