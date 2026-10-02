#!/usr/bin/env python3
"""Text checks over the repository (A15.12; A15.12's `kd check file` and `kd check ids` as this script's modes).

Modes now:  note                 dist/NOTE.md has its sections and the APK link (PRC-11)
            gate <description>   the merge gate of A15.13 step 6 (PRC-09)
The file, commit and coverage modes arrive in α00b. Python 3.11 standard library only.
"""
import json
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
NOTE_HEADINGS = ["What is new", "What to try", "What is rough", "IDs delivered", "Links"]
SESSION = re.compile(r"session_[A-Za-z0-9]+")


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


def git(*args):
    return subprocess.run(["git", "-C", ROOT, *args], check=True, capture_output=True, text=True).stdout


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
