#!/usr/bin/env python3
"""Text checks over the repository (A15.12; A15.12's `kd check file` and `kd check ids` as this script's modes).

Modes:  note                 dist/NOTE.md has its sections and the APK link (PRC-11)
        gate <description>   the merge gate of A15.13 step 6 (PRC-09)
        file                 the file check on the three documents and the commit check (PRC-10, PRC-07; A15.12 step 7)
Python 3.11 standard library only.
"""
import difflib
import json
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
NOTE_HEADINGS = ["What is new", "What to try", "What is rough", "IDs delivered", "Links"]
SESSION = re.compile(r"session_[A-Za-z0-9]+")

# The file check (PRC-10) and the commit check (PRC-07).
ID = r"[A-Z]{3}-\d{2,3}"
ID_RE = re.compile(r"\b" + ID + r"\b")
STATUSES = ("Decided", "Proposed", "To test", "Dropped")
# A line that sets out to be an item marker: a list entry opening with a backticked token and a bold name.
CANDIDATE = re.compile(r"^\s*(?:-|\d+\.)\s+`([^`]+)`\s+\*\*")
MARKER = re.compile(r"^\s*(?:- |\d+\. )`(" + ID + r")` \*\*(.+?)\*\* \*\(([^)]*)\)\*(?::.*)?$")
SPAN = re.compile(r"`([^`\n]+)`")
SECTION = re.compile(r"(?<![\w.#/-])(A\d{1,2}(?:\.\d{1,2})?)(?!\w)")
ARCH_HEADING = re.compile(r"^#{1,6}\s+(A\d+(?:\.\d+)*)\.?\s", re.M)
SKIPPED_SECTION = "## How this file works"  # examples, not references
ALPHA_FIELDS = ["Goal", "Serves", "Architecture", "Needs", "Crates and files touched", "Tasks", "Data", "Tests",
                "On the phone", "Not in this alpha", "Risks"]
FIELD = re.compile(r"^\*\*([A-Z][^*:]*?):\*\*", re.M)
TASK_LINE = re.compile(r"^\d+\. `([^`]+)`")
KNOWN_FILE = "tools/filecheck-known.txt"


def check_note(text):
    """Problems with a note's text: its five headings and a link ending dist/kindling.apk (PRC-11)."""
    problems = []
    headings = {m.group(1).strip() for m in re.finditer(r"^#{1,6}\s+(.+)$", text, re.M)}
    for h in NOTE_HEADINGS:
        if h not in headings:
            problems.append(f"heading missing: {h}")
    if not re.search(r"\]\(\s*[^)\s]*dist/kindling\.apk\s*\)", text):
        problems.append("no link ending dist/kindling.apk")
    return problems


def gate(description, head, head_only_results, head_passed, parent_passed, trailer_sessions):
    """Problems with a merge (A15.13 step 6).

    description: the pull request's description; head: the head commit's full hash;
    head_only_results: True when the head changes only results/; head_passed, parent_passed: results/checks says PASS
    for the head, and for its parent; trailer_sessions: every Claude-Session trailer on the branch.
    """
    m = re.search(r"^\s*Review: APPROVE\s+([0-9a-f]{7,40})\s+(\S+)", description, re.M)
    if not m:
        return ["no 'Review: APPROVE <commit> <session>' line"]
    problems = []
    commit, reviewer = m.group(1), m.group(2)
    if len(commit) < 12 or commit[:12] != head[:12]:
        problems.append(f"approved commit {commit} is not the head {head[:12]}")
    head12 = head[:12]
    if not (head_passed or (head_only_results and parent_passed)):
        problems.append(f"no passing results/checks for {head12} or, with a results-only head, its parent")
    rs = set(SESSION.findall(reviewer))
    if not rs:
        problems.append(f"reviewer session {reviewer} names no session")
    builders = set()
    for t in trailer_sessions:
        builders.update(SESSION.findall(t))
    if rs & builders:
        problems.append(f"the reviewer's session {reviewer} also built this branch")
    return problems


class Item:
    """One PROJECT.md item: its ID, name, status and its own lines (0-based, marker to the next item or heading)."""

    def __init__(self, id_, name, status, start):
        self.id, self.name, self.status, self.start, self.end = id_, name, status, start, start


def cited_ids(text, known_areas):
    """(line from 1, ID) for every ID cited in a text: anything shaped like one inside backticks, and outside them
    only IDs of a known area, so `SHA-256` in prose is not taken for one."""
    out = []
    for n, line in enumerate(text.split("\n"), 1):
        for m in SPAN.finditer(line):
            out += [(n, x) for x in ID_RE.findall(m.group(1))]
        bare = SPAN.sub(" ", line)
        out += [(n, x) for x in ID_RE.findall(bare) if x[:3] in known_areas]
    return out


def blank_skipped(lines):
    """PROJECT.md's lines with the section `## How this file works` blanked (its IDs are examples)."""
    out, skip = [], False
    for line in lines:
        if line.startswith("## "):
            skip = line.strip() == SKIPPED_SECTION
        out.append("" if skip else line)
    return out


def parse_project(text):
    """PROJECT.md's items by ID, in order, and the problems with their markers (PRC-10's file check).
    Implements PRC-10, see A15.12 step 7."""
    lines = blank_skipped(text.split("\n"))
    items, problems, current = {}, [], None
    for i, line in enumerate(lines):
        if line.startswith("#"):
            current = None
            continue
        c = CANDIDATE.match(line)
        if not c:
            if current:
                current.end = i
            continue
        m = MARKER.match(line)
        if not m:
            token = c.group(1)
            if not re.fullmatch(ID, token):
                problems.append(f"PROJECT.md line {i + 1}: `{token}` is not an ID (three capitals, a dash, 2 or 3 digits)")
            else:
                problems.append(f"PROJECT.md line {i + 1}: `{token}`'s marker does not parse as `ID` **Name** *(Status)*")
            current = None
            continue
        id_, name, status = m.group(1), m.group(2), m.group(3)
        if status not in STATUSES:
            problems.append(f"PROJECT.md line {i + 1}: `{id_}` has status *{status}*, not one of {', '.join(STATUSES)}")
        if id_ in items:
            problems.append(f"PROJECT.md line {i + 1}: `{id_}` defined twice (first on line {items[id_].start + 1})")
            current = None
            continue
        current = items[id_] = Item(id_, name, status, i)
    return items, problems


def check_project(text, items):
    """Every cited ID resolves, and no live item cites a dropped one in its own lines (PRC-10)."""
    lines = blank_skipped(text.split("\n"))
    areas = {i[:3] for i in items}
    problems, count = [], 0
    for n, x in cited_ids("\n".join(lines), areas):
        count += 1
        if x not in items:
            problems.append(f"PROJECT.md line {n}: `{x}` is not defined")
    for it in items.values():
        if it.status == "Dropped":
            continue
        own = "\n".join(lines[it.start:it.end + 1])
        for n, x in cited_ids(own, areas):
            if x in items and items[x].status == "Dropped":
                problems.append(f"PROJECT.md line {it.start + n}: live `{it.id}` cites dropped `{x}`")
    return problems, count


def arch_sections(arch_text):
    """The section numbers that are headings in ARCHITECTURE.md (`A5`, `A5.3`)."""
    return set(ARCH_HEADING.findall(arch_text))


def check_citations(name, text, items, sections, live_only):
    """Every ID a document cites exists (and, with live_only, is not Dropped); every section it cites is a heading
    in ARCHITECTURE.md (PRC-10)."""
    problems, count = [], 0
    areas = {i[:3] for i in items}
    for n, x in cited_ids(text, areas):
        count += 1
        if x not in items:
            problems.append(f"{name} line {n}: `{x}` is not defined in PROJECT.md")
        elif live_only and items[x].status == "Dropped":
            problems.append(f"{name} line {n}: `{x}` is Dropped")
    for n, line in enumerate(text.split("\n"), 1):
        if line.startswith("#") and name == "ARCHITECTURE.md":
            continue  # a heading defines its section rather than citing it
        for s in SECTION.findall(line):
            count += 1
            if s not in sections:
                problems.append(f"{name} line {n}: section {s} is not a heading in ARCHITECTURE.md")
    return problems, count


def alpha_sections(plan_text):
    """(alpha code such as `00b`, heading line from 1, body) for each `### α..` section of the plan."""
    lines = plan_text.split("\n")
    out, i = [], 0
    while i < len(lines):
        m = re.match(r"^### α(\d+[a-z]?)\b", lines[i])
        if not m:
            i += 1
            continue
        j = i + 1
        while j < len(lines) and not re.match(r"^#{1,3} ", lines[j]):
            j += 1
        out.append((m.group(1), i + 1, "\n".join(lines[i + 1:j])))
        i = j
    return out


def check_plan_layout(plan_text):
    """Each alpha section holds the eleven field labels in order, and its numbered task lines open with unique task
    IDs that match the alpha (`T00b.2` in α00b) (PRC-10)."""
    problems, seen = [], {}
    for code, line, body in alpha_sections(plan_text):
        fields = [f for f in FIELD.findall(body) if f in ALPHA_FIELDS]
        if fields != ALPHA_FIELDS:
            problems.append(f"IMPLEMENTATION.md line {line}: α{code}'s field labels are {fields}, not the template's "
                            f"{len(ALPHA_FIELDS)} in order")
        for k, l in enumerate(body.split("\n"), line + 1):
            m = TASK_LINE.match(l)
            if not m:
                continue
            t = m.group(1)
            if not re.fullmatch(rf"T{code}\.\d+", t):
                problems.append(f"IMPLEMENTATION.md line {k}: task `{t}` does not match α{code} (T{code}.<n>)")
            if t in seen:
                problems.append(f"IMPLEMENTATION.md line {k}: task `{t}` repeats line {seen[t]}")
            seen.setdefault(t, k)
    return problems


def item_at(items, line):
    """The ID of the item whose lines hold a 0-based line, or None."""
    for it in items.values():
        if it.start <= line <= it.end:
            return it.id
    return None


def changed_items(old_text, new_text):
    """The IDs whose item lines differ between two versions of PROJECT.md."""
    old_items, _ = parse_project(old_text)
    new_items, _ = parse_project(new_text)
    old, new = old_text.split("\n"), new_text.split("\n")
    ids = set()
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, old, new, autojunk=False).get_opcodes():
        if tag == "equal":
            continue
        ids.update(item_at(old_items, i) for i in range(i1, i2))
        ids.update(item_at(new_items, j) for j in range(j1, j2))
    ids.discard(None)
    return ids


def check_commit(sha, message, old_text, new_text):
    """A commit changing PROJECT.md carries a `Changed:` line naming every ID whose item lines it changed (PRC-07).
    Implements PRC-07, see A15.12 step 7."""
    changed_lines = [l for l in message.split("\n") if l.strip().startswith("Changed:")]
    if not changed_lines:
        return [f"commit {sha[:12]} changes PROJECT.md without a `Changed:` line (PRC-07)"]
    named = set(ID_RE.findall("\n".join(changed_lines)))
    missing = sorted(changed_items(old_text, new_text) - named)
    if missing:
        return [f"commit {sha[:12]} changes {', '.join(missing)} in PROJECT.md without naming them in `Changed:`"]
    return []


def read_known(path):
    """Findings awaiting the owner's OK, one a line: `<finding> | <date> | awaiting owner OK` (`#` starts a comment)."""
    known = set()
    if os.path.exists(path):
        for line in open(path, encoding="utf-8"):
            line = line.strip()
            if line and not line.startswith("#"):
                known.add(line.split(" | ")[0].strip())
    return known


def file_check(project, arch, plan, commits):
    """The whole file check over the three documents and the commits (sha, message, old and new PROJECT.md).
    Returns (problems, item count, citation count)."""
    items, problems = parse_project(project)
    p, c1 = check_project(project, items)
    problems += p
    sections = arch_sections(arch)
    p, c2 = check_citations("ARCHITECTURE.md", arch, items, sections, live_only=False)
    problems += p
    p, c3 = check_citations("IMPLEMENTATION.md", plan, items, sections, live_only=True)
    problems += p + check_plan_layout(plan)
    for sha, message, old, new in commits:
        problems += check_commit(sha, message, old, new)
    return problems, len(items), c1 + c2 + c3


def git(*args):
    return subprocess.run(["git", "-C", ROOT, *args], check=True, capture_output=True, text=True).stdout


def show(rev, path):
    """A file at a revision, or "" where it does not exist."""
    r = subprocess.run(["git", "-C", ROOT, "show", f"{rev}:{path}"], capture_output=True, text=True)
    return r.stdout if r.returncode == 0 else ""


def branch_commits(path):
    """(sha, message, old text, new text) for each commit in origin/main..HEAD, merges left out, changing a file."""
    shas = git("log", "--no-merges", "--format=%H", "origin/main..HEAD", "--", path).split()
    return [(s, git("log", "-1", "--format=%B", s), show(f"{s}^", path), show(s, path)) for s in shas]


def read(path):
    with open(os.path.join(ROOT, path), encoding="utf-8") as f:
        return f.read()


def report(prefix, problems, known_path, ok_line):
    """Prints the problems not awaiting the owner, then the verdict; returns the exit code."""
    known = read_known(known_path)
    left = [p for p in problems if p not in known]
    for p in sorted(known & set(problems)):
        print(f"{prefix}: known, awaiting owner OK: {p}")
    for p in sorted(known - set(problems)):
        print(f"{prefix}: info: {known_path} lists a finding that no longer occurs: {p}")
    for p in left:
        print(f"{prefix}: {p}")
    print(ok_line if not left else f"{prefix}: FAIL ({len(left)} problems)")
    return 0 if not left else 1


def result_passes(commit12):
    path = os.path.join(ROOT, "results", "checks", f"{commit12}.json")
    if not os.path.exists(path):
        return False
    with open(path) as f:
        r = json.load(f)
    return r.get("result") == "PASS" and r.get("commit") == commit12


def main(argv):
    if len(argv) >= 1 and argv[0] == "note":
        path = os.path.join(ROOT, "dist", "NOTE.md")
        problems = check_note(open(path, encoding="utf-8").read()) if os.path.exists(path) else ["dist/NOTE.md missing"]
        for p in problems:
            print(f"Note: {p}")
        print("Note: PASS" if not problems else "Note: FAIL")
        return 0 if not problems else 1
    if argv[:1] == ["file"]:
        try:
            git("rev-parse", "--verify", "-q", "origin/main")
        except subprocess.CalledProcessError:
            print("File check: origin/main is not fetched (git fetch origin +refs/heads/main:refs/remotes/origin/main)")
            return 1
        commits = branch_commits("PROJECT.md")
        problems, n, m = file_check(read("PROJECT.md"), read("ARCHITECTURE.md"), read("IMPLEMENTATION.md"), commits)
        ok = f"File check: OK ({n} items, {m} citations, {len(commits)} commits changing PROJECT.md)"
        return report("File check", problems, os.path.join(ROOT, KNOWN_FILE), ok)
    if len(argv) >= 2 and argv[0] == "gate":
        description = open(argv[1], encoding="utf-8").read()
        head = git("rev-parse", "HEAD").strip()
        parent = git("rev-parse", "HEAD^").strip()
        files = [f for f in git("diff", "--name-only", "HEAD^", "HEAD").split("\n") if f]
        only_results = bool(files) and all(f.startswith("results/") for f in files)
        trailers = git("log", "--format=%(trailers:key=Claude-Session,valueonly)", "origin/main..HEAD").split("\n")
        sessions = [t for t in trailers if t.strip()]
        problems = gate(description, head, only_results, result_passes(head[:12]), result_passes(parent[:12]), sessions)
        for p in problems:
            print(f"Gate: {p}")
        if problems:
            print("Gate: FAIL")
            return 1
        print(f"Gate: PASS {head[:12]}")
        return 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
