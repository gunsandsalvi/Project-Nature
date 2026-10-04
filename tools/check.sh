#!/usr/bin/env bash
# The checks before work joins main (PRC-10, A17), in order, stopping at the first failure:
#   1 formats     GDScript (gdformat), C++ (clang-format) and Python (ruff)
#   2 lints       GDScript (gdlint), Python (ruff) and shell (bash -n); C++'s with its build, in 3
#   3 C++         each CMake project built, its doctest tests run, its code linted (clang-tidy)
#   4 Godot       each Godot project imported, every script compiled, and its gdUnit4 tests run headless
#   5 tools       the tool tests, and the self-tests of the file check and the signing key
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
[ -x "$GODOT" ] && command -v gdformat >/dev/null && command -v ruff >/dev/null && [ -d "$KD_GDUNIT" ] || tools/setup.sh
COMMIT="$(git rev-parse --short=12 HEAD)"
T0=$(date +%s)
git fetch -q origin +refs/heads/main:refs/remotes/origin/main 2>/dev/null || true
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
step() { echo "== $*"; }
quiet() { "$@" >"$TMP/out" 2>&1 || { cat "$TMP/out"; return 1; }; }

# Our code: every file git holds or would hold, outside third-party code. The bake-off is kept as it was, for P1
# (research 01).
ALL=()
while IFS= read -r f; do [ -f "$f" ] && ALL+=("$f"); done < <(git ls-files -co --exclude-standard \
  | grep -vE '^prototypes/bakeoff/|(^|/)(addons|thirdparty|godot-cpp)/' | sort -u)
pick() { printf '%s\n' "${ALL[@]}" | grep -E "\.($1)\$" || true; }
mapfile -t GD < <(pick gd)
mapfile -t CPP < <(pick 'cpp|cc|h|hpp')
mapfile -t PY < <(pick py)
mapfile -t SH < <(pick sh)

step "1 formats"
[ "${#GD[@]}" -eq 0 ] || quiet gdformat --check "${GD[@]}"
[ "${#CPP[@]}" -eq 0 ] || quiet clang-format --dry-run --Werror "${CPP[@]}"
[ "${#PY[@]}" -eq 0 ] || quiet ruff format --check "${PY[@]}"
echo "   ${#GD[@]} GDScript, ${#CPP[@]} C++ and ${#PY[@]} Python files"

step "2 lints"
[ "${#GD[@]}" -eq 0 ] || quiet gdlint "${GD[@]}"
[ "${#PY[@]}" -eq 0 ] || quiet ruff check "${PY[@]}"
for f in "${SH[@]}"; do quiet bash -n "$f"; done
echo "   GDScript, Python and ${#SH[@]} shell scripts"

step "3 C++"
CMAKE=()
for d in sim view prototypes/*; do
  [ -f "$d/CMakeLists.txt" ] && [ "$d" != prototypes/bakeoff ] && CMAKE+=("$d")
done
[ "${#CMAKE[@]}" -gt 0 ] || echo "   no C++ projects yet"
for d in "${CMAKE[@]}"; do
  B="build/${d//\//-}"
  quiet cmake -S "$d" -B "$B" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  quiet cmake --build "$B"
  quiet ctest --test-dir "$B" --output-on-failure
  mapfile -t SRC < <(printf '%s\n' "${CPP[@]}" | grep -E "^$d/.*\.(cpp|cc)\$" || true)
  [ "${#SRC[@]}" -eq 0 ] || quiet clang-tidy -p "$B" --quiet --header-filter="^$ROOT/$d/" "${SRC[@]}"
  echo "   $d: built, $(ctest --test-dir "$B" -N | sed -n 's/^Total Tests: //p') tests passed, ${#SRC[@]} files linted"
done
echo "   the same results on x86-64 and arm64, and on one thread and four (A3.4): from α0.4a"

step "4 Godot"
PROJECTS=()
for f in game/project.godot prototypes/*/project.godot; do [ -f "$f" ] && PROJECTS+=("$(dirname "$f")"); done
[ "${#PROJECTS[@]}" -gt 0 ] || echo "   no Godot projects yet"
for d in "${PROJECTS[@]}"; do
  before="$(git status --porcelain --untracked-files=all -- "$d")"
  rm -rf "$d/addons/gdUnit4"
  mkdir -p "$d/addons"
  cp -r "$KD_GDUNIT/addons/gdUnit4" "$d/addons/gdUnit4"
  timeout 600 "$GODOT" --headless --path "$d" --import >"$TMP/import" 2>&1 || { cat "$TMP/import"; exit 1; }
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
  # Godot writes a .uid file beside each new script and shader: they belong in the commit, so the check stops
  # when Godot has added or changed anything.
  after="$(git status --porcelain --untracked-files=all -- "$d")"
  if [ "$before" != "$after" ]; then
    echo "Godot wrote files in $d; commit them with the step:"
    diff <(echo "$before") <(echo "$after") | sed -n 's/^> /   /p'
    exit 1
  fi
  echo "   $d: imported, $COMPILED scripts compiled, $RAN gdUnit4 tests passed"
done

step "5 tools"
python3 -m unittest discover -s tools/tests >"$TMP/unit" 2>&1 || { cat "$TMP/unit"; exit 1; }
echo "   $(sed -n 's/^Ran \([0-9]*\) tests.*/\1/p' "$TMP/unit") tool tests passed"
quiet python3 tools/signing-key.py selftest
SELF="$(python3 tools/filecheck.py selftest)" || { echo "$SELF"; exit 1; }
echo "   ${SELF##*$'\n'}"

step "6 file check"
python3 tools/filecheck.py file

step "7 coverage"
python3 tools/filecheck.py ids --merge

if [ "$DELIVER" = 1 ]; then
  step "8 delivery"
  python3 tools/filecheck.py note
  # The step the note delivers, from its title, such as "# α0.1a The workshop".
  STEP="$(sed -n '1s/^# α\([0-9]*\.[0-9]*[a-e]\).*/\1/p' dist/NOTE.md)"
  [ -n "$STEP" ] || { echo "Delivery: dist/NOTE.md's title names no step, such as '# α0.1a The workshop'"; exit 1; }
  tools/build.sh "$STEP" check
  (cd dist && sha256sum --quiet -c kindling.apk.sha256)
  tools/verify-apk.sh dist/kindling.apk release "$STEP"
fi

SECS=$(($(date +%s) - T0))
if [ "$SECS" -lt 120 ]; then TOOK="$SECS seconds"; else TOOK="$(((SECS + 59) / 60)) minutes"; fi
echo "Checks: PASS $COMMIT ($TOOK)"
