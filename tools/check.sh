#!/usr/bin/env bash
# The checks before work joins main (PRC-10, A17), in order, stopping at the first failure:
#   1 formats     GDScript (gdformat), C++ (clang-format 18) and Python (ruff)
#   2 lints       GDScript (gdlint), Python (ruff) and shell (bash -n); C++'s with its build, in 3
#   3 C++         each CMake project built, through ccache; then, beside steps 4 and 5, its doctest tests run (with
#                 the same results on x86-64 and on arm64 under qemu, on one thread and four) and its code linted
#                 (clang-tidy 18), each only when what it depends on changed since it passed (tools/cppcache.py)
#   4 Godot       each Godot project imported, every script compiled, and its gdUnit4 tests run headless
#   5 tools       the tool tests, and the self-tests of the file check and the signing key, after step 4
#   6 file check  the three documents, and every commit since main that changes PROJECT.md (PRC-07)
#   7 coverage    every item mapped and every test naming what it checks (PRC-12)
#   8 delivery    with --deliver: the note, the build signed with a key made for it and checked, and the committed APK
# Usage: tools/check.sh [--deliver]
# It ends with "Checks: PASS <commit>"; nothing is committed for it, and what it builds stays in ignored folders.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
. tools/env.sh
DELIVER=0
case "${1:-}" in
  "") ;;
  --deliver) DELIVER=1 ;;
  *) echo "usage: tools/check.sh [--deliver]" >&2; exit 2 ;;
esac
[ -x "$GODOT" ] && command -v gdformat >/dev/null && command -v ruff >/dev/null && command -v ccache >/dev/null \
  && [ -d "$KD_GDUNIT" ] || tools/setup.sh
COMMIT="$(git rev-parse --short=12 HEAD)"
T0=$(date +%s)
git fetch -q origin +refs/heads/main:refs/remotes/origin/main 2>/dev/null || true
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
# Each step's header, after the time the step before took, so a slow step shows.
STEP_T=$(date +%s)
step() {
  [ -z "${STEP_NAME:-}" ] || echo "   took $(($(date +%s) - STEP_T)) s"
  STEP_T=$(date +%s)
  STEP_NAME="$*"
  echo "== $*"
}
quiet() {
  local out
  out="$(mktemp -p "$TMP")"
  "$@" >"$out" 2>&1 || { cat "$out"; return 1; }
}
# A fingerprint of the named files' names and contents, and of any words among them that are not files, in any order,
# for the steps skipped while nothing they read changed since they passed (A17); the passes are kept in build/passed/.
fingerprint() {
  local f
  for f in "$@"; do
    if [ -f "$f" ]; then sha256sum "$f"; else echo "word $f"; fi
  done | sort | sha256sum | cut -c1-64
}
passed() { [ "$(cat "build/passed/$1" 2>/dev/null)" = "$2" ]; }
pass() {
  mkdir -p build/passed
  echo "$2" >"build/passed/$1"
}

# Our code: every file git holds or would hold, outside third-party code.
ALL=()
while IFS= read -r f; do [ -f "$f" ] && ALL+=("$f"); done < <(git ls-files -co --exclude-standard \
  | grep -vE '(^|/)(addons|thirdparty|godot-cpp)/' | sort -u)
pick() { printf '%s\n' "${ALL[@]}" | grep -E "\.($1)\$" || true; }
mapfile -t GD < <(pick gd)
mapfile -t CPP < <(pick 'cpp|cc|h|hpp')
mapfile -t PY < <(pick py)
mapfile -t SH < <(pick sh)

step "1 formats"
[ "${#GD[@]}" -eq 0 ] || quiet gdformat --check "${GD[@]}"
[ "${#CPP[@]}" -eq 0 ] || quiet clang-format-18 --dry-run --Werror "${CPP[@]}"
[ "${#PY[@]}" -eq 0 ] || quiet ruff format --check "${PY[@]}"
echo "   ${#GD[@]} GDScript, ${#CPP[@]} C++ and ${#PY[@]} Python files"

step "2 lints"
[ "${#GD[@]}" -eq 0 ] || quiet gdlint "${GD[@]}"
[ "${#PY[@]}" -eq 0 ] || quiet ruff check "${PY[@]}"
for f in "${SH[@]}"; do quiet bash -n "$f"; done
echo "   GDScript, Python and ${#SH[@]} shell scripts"

step "3 C++"
CMAKE=()
for d in sim view; do
  [ -f "$d/CMakeLists.txt" ] && CMAKE+=("$d")
done
[ "${#CMAKE[@]}" -gt 0 ] || echo "   no C++ projects yet"
for d in "${CMAKE[@]}"; do
  B="build/${d//\//-}"
  quiet cmake -S "$d" -B "$B" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
  quiet cmake --build "$B"
done
echo "   ${#CMAKE[@]} projects built"

# Each C++ project's doctest tests, then its code linted (clang-tidy 18).
cpp_tests() {
  echo "== 3 C++ tests"
  for d in "${CMAKE[@]}"; do
    B="build/${d//\//-}"
    # the tests run again only when something they are built from changed since they passed (tools/cppcache.py)
    STAMP="$(python3 tools/cppcache.py tests "$B")"
    if [ "$STAMP" != unknown ] && [ "$(cat "$B/tests.passed" 2>/dev/null)" = "$STAMP" ]; then
      TESTS="tests unchanged since they passed"
    else
      quiet ctest --test-dir "$B" --output-on-failure -j "$(nproc)"
      [ "$STAMP" = unknown ] || echo "$STAMP" >"$B/tests.passed"
      TESTS="$(ctest --test-dir "$B" -N | sed -n 's/^Total Tests: //p') tests passed"
    fi
    # and the lint, on every core, only the files whose code, headers, compile command or rules changed
    mapfile -t SRC < <(printf '%s\n' "${CPP[@]}" | grep -E "^$d/.*\.(cpp|cc)\$" || true)
    LINT="no files to lint"
    if [ "${#SRC[@]}" -gt 0 ]; then
      LINT="$(python3 tools/cppcache.py lint "$B" "^$ROOT/$d/" "${SRC[@]}")" || { echo "$LINT"; exit 1; }
    fi
    echo "   $d: $TESTS; lint: $LINT"
  done
}

godot_step() {
  echo "== 4 Godot"
  PROJECTS=()
  [ ! -f game/project.godot ] || PROJECTS+=(game)
  [ "${#PROJECTS[@]}" -gt 0 ] || echo "   no Godot projects yet"
  for d in "${PROJECTS[@]}"; do
    # what the import, the scripts and the tests read: the project's files, the extensions built for this machine,
    # and the tools' versions
    mapfile -t READS < <(printf '%s\n' "${ALL[@]}" | grep "^$d/"; compgen -G "$d/*/bin/*.so" || true)
    FP="$(fingerprint "${READS[@]}" tools/godot-scripts.gd "$KD_GODOT_VERSION" "$KD_GDUNIT")"
    before="$(git status --porcelain --untracked-files=all -- "$d")"
    if passed "godot-${d//\//-}" "$FP"; then
      SUMMARY="unchanged since its import, scripts and gdUnit4 tests passed"
    else
      rm -rf "$d/addons/gdUnit4"
      mkdir -p "$d/addons"
      cp -r "$KD_GDUNIT/addons/gdUnit4" "$d/addons/gdUnit4"
      # Godot 4.7 can abort as it exits after importing new files, their import done (its Android plug-in finds no adb
      # daemon here); a second import, with nothing left to do, exits cleanly
      timeout 600 "$GODOT" --headless --path "$d" --import >"$TMP/import" 2>&1 \
        || timeout 600 "$GODOT" --headless --path "$d" --import >>"$TMP/import" 2>&1 || { cat "$TMP/import"; exit 1; }
      if grep -E 'SCRIPT ERROR|Parse Error|Failed to load script' "$TMP/import"; then exit 1; fi
      timeout 300 "$GODOT" --headless --path "$d" -s "$ROOT/tools/godot-scripts.gd" >"$TMP/scripts" 2>&1 \
        || { grep -vE '^\s*$' "$TMP/scripts" | tail -40; exit 1; }
      COMPILED="$(sed -n 's/^Scripts: \([0-9]*\) compiled.*/\1/p' "$TMP/scripts")"
      RAN=0
      if [ -d "$d/test" ]; then
        # Only exit code 0 passes: 100 is a failure, 101 a node a test left behind, 105 a script error. The reports go
        # to Godot's user folder, outside the repository; the remote-debug address keeps Godot's debugger from waiting
        # for input after a script error.
        rc=0
        timeout 600 "$GODOT" --headless --path "$d" -s -d --remote-debug tcp://127.0.0.1:0 \
          res://addons/gdUnit4/bin/GdUnitCmdTool.gd --ignoreHeadlessMode -a res://test -rd user://gdunit-reports -c \
          >"$TMP/tests" 2>&1 || rc=$?
        sed 's/\x1b\[[0-9;]*m//g' "$TMP/tests" >"$TMP/plain"
        if [ "$rc" -ne 0 ]; then
          grep -vE '^\s*$' "$TMP/plain" | tail -60
          echo "gdUnit4 in $d: exit $rc"
          exit 1
        fi
        RAN="$(sed -n 's/^Overall Summary: \([0-9]*\) test cases.*/\1/p' "$TMP/plain")"
        [ -n "$RAN" ] || { tail -40 "$TMP/plain"; echo "gdUnit4 in $d: no summary"; exit 1; }
      fi
      SUMMARY="imported, $COMPILED scripts compiled, $RAN gdUnit4 tests passed"
      PASSED_GODOT="$FP"
    fi
    # Godot writes a .uid file beside each new script and shader: they belong in the commit, so the check stops
    # when Godot has added or changed anything, when a committed script's .uid is not committed with it, and, for a
    # delivery, when anything in the project is not committed at all.
    after="$(git status --porcelain --untracked-files=all -- "$d")"
    if [ "$before" != "$after" ]; then
      echo "Godot wrote files in $d; commit them with the step:"
      diff <(echo "$before") <(echo "$after") | sed -n 's/^> /   /p'
      exit 1
    fi
    for f in $(git ls-files -- "$d" | grep -E '\.(gd|gdshader|gdshaderinc)$' || true); do
      git ls-files --error-unmatch "$f.uid" >/dev/null 2>&1 || { echo "$f.uid is not committed with $f"; exit 1; }
    done
    if [ "$DELIVER" = 1 ] && [ -n "$after" ]; then
      echo "Not committed in $d, which a delivery needs:"
      sed 's/^/   /' <<<"$after"
      exit 1
    fi
    [ -z "${PASSED_GODOT:-}" ] || pass "godot-${d//\//-}" "$PASSED_GODOT"
    PASSED_GODOT=""
    echo "   $d: $SUMMARY"
  done
}

tools_step() {
  echo "== 5 tools"
  python3 -m unittest discover -s tools/tests >"$TMP/unit" 2>&1 || { cat "$TMP/unit"; exit 1; }
  echo "   $(sed -n 's/^Ran \([0-9]*\) tests.*/\1/p' "$TMP/unit") tool tests passed"
  quiet python3 tools/signing-key.py selftest
  SELF="$(python3 tools/filecheck.py selftest)" || { echo "$SELF"; exit 1; }
  echo "   ${SELF##*$'\n'}"
}

# The C++ tests run beside the Godot and tool steps, which need only what 3 built; the tool step follows Godot's, since
# a tool test may run Godot on the same project, whose import cache two runs at once would race on. Each side writes
# its own log, shown in order once both are done.
side() {
  local t
  t=$(date +%s)
  for s in "$@"; do "$s"; done
  echo "   took $(($(date +%s) - t)) s"
}
step_done() {
  echo "   took $(($(date +%s) - STEP_T)) s"
  STEP_NAME=""
}
step_done
(side cpp_tests) >"$TMP/side-a" 2>&1 &
A=$!
(side godot_step tools_step) >"$TMP/side-b" 2>&1 &
B=$!
RA=0
RB=0
wait "$A" || RA=$?
wait "$B" || RB=$?
cat "$TMP/side-a" "$TMP/side-b"
[ "$RA" -eq 0 ] && [ "$RB" -eq 0 ] || exit 1
STEP_T=$(date +%s)

step "6 file check"
python3 tools/filecheck.py file

step "7 coverage"
python3 tools/filecheck.py ids --merge

if [ "$DELIVER" = 1 ]; then
  step "8 delivery"
  python3 tools/filecheck.py note
  # The step the note delivers, from its title, such as "# Kindling α0.1a: The workshop".
  STEP="$(sed -n '1s/^# .*α\([0-9]\{1,2\}\.[0-9]\{1,2\}[a-e]\).*/\1/p' dist/NOTE.md)"
  [ -n "$STEP" ] || { echo "Delivery: dist/NOTE.md's title names no step, such as 'α0.1a'"; exit 1; }
  tools/build.sh "$STEP" check
  (cd dist && sha256sum --quiet -c kindling.apk.sha256)
  tools/verify-apk.sh dist/kindling.apk release "$STEP"
fi

SECS=$(($(date +%s) - T0))
if [ "$SECS" -lt 120 ]; then TOOK="$SECS seconds"; else TOOK="$(((SECS + 59) / 60)) minutes"; fi
echo "   took $(($(date +%s) - STEP_T)) s"
echo "Checks: PASS $COMMIT ($TOOK)"
