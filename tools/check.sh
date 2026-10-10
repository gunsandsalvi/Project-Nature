#!/usr/bin/env bash
# Routine merge/delivery checks (PRC-10, A17); --audit adds the expensive foundation audit.
# Usage: tools/check.sh [--deliver] [--audit]
# Routine: formats/lints/native source rules, host builds/tests, M1 proofs on one/four threads, catalogue,
# Godot import/scripts/tests, tools, documents/IDs, and the existing signed APK for delivery.
# Audit: cross-compiler digests, TSan, shuffled ties and kill/scenes/repeat only
# (owner, 10 October 2026). No emulated full suites, render benchmark or second export.
# Ends with "Checks: PASS <commit>"; generated files stay in ignored folders.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
. tools/env.sh
DELIVER=0
AUDIT=0
for option in "$@"; do
  case "$option" in
    --deliver) DELIVER=1 ;;
    --audit) AUDIT=1 ;;
    *) echo "usage: tools/check.sh [--deliver] [--audit]" >&2; exit 2 ;;
  esac
done
COMMIT="$(git rev-parse --short=12 HEAD)"
T0=$(date +%s)
git fetch -q origin +refs/heads/main:refs/remotes/origin/main 2>/dev/null || true
TMP="$(mktemp -d)"
trap '[ -z "${AUDIT_THREAD_PID:-}" ] || kill "$AUDIT_THREAD_PID" 2>/dev/null || true; rm -rf "$TMP"' EXIT
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

build_one() {  # name, source folder, configure options...; TARGET, if set, builds only that target
  local name="$1" src="$2"
  shift 2
  quiet cmake -S "$src" -B "build/$name" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache "$@"
  quiet cmake --build "build/$name" ${TARGET:+--target "$TARGET"}
}
# The audit adds only checks with distinct failure modes (owner, 10 October 2026).
# Binary/data/helper fingerprints retain passes; changed inputs rerun the affected checks.
audit() {
  local san="-fsanitize=undefined -fsanitize=float-cast-overflow -fno-sanitize-recover=all"
  local names=(sim sim-gcc sim-a64-ndk sim-a64-tie1 sim-a64-tie2)
  local ndk=("-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake"
    -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-24 -DANDROID_STL=c++_static
    -DCMAKE_EXE_LINKER_FLAGS=-static -DCMAKE_CROSSCOMPILING_EMULATOR=qemu-aarch64-static)
  step "audit builds"
  TARGET=kindling build_one sim sim -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
  TARGET=kindling build_one sim-gcc sim -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
    "-DCMAKE_C_FLAGS=$san" "-DCMAKE_CXX_FLAGS=$san" "-DCMAKE_EXE_LINKER_FLAGS=$san"
  TARGET=kindling build_one sim-a64-ndk sim "${ndk[@]}"
  TARGET=kd_sim_tests build_one sim-tsan sim -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_C_FLAGS=-fsanitize=thread -DCMAKE_CXX_FLAGS=-fsanitize=thread -DCMAKE_EXE_LINKER_FLAGS=-fsanitize=thread
  for seed in 1 2; do
    TARGET=kindling build_one "sim-a64-tie$seed" sim "${ndk[@]}" \
      "-DCMAKE_CXX_FLAGS=-D_LIBCPP_DEBUG_RANDOMIZE_UNSPECIFIED_STABILITY -D_LIBCPP_DEBUG_RANDOMIZE_UNSPECIFIED_STABILITY_SEED=$seed"
  done
  local thread_stamp thread_pid=""
  thread_stamp="$(python3 tools/cppcache.py tests build/sim-tsan)"
  if [ "$thread_stamp" = unknown ] || [ "$(cat build/sim-tsan/tests.passed 2>/dev/null)" != "$thread_stamp" ]; then
    TSAN_OPTIONS=halt_on_error=1 build/sim-tsan/kd_sim_tests >"$TMP/threads" 2>&1 &
    thread_pid=$!
    AUDIT_THREAD_PID=$thread_pid
  else
    echo "   TSan unchanged since it passed"
  fi
  step "cross-compiler digests and shuffled ties"
  local data_files=() binaries=() fp run=()
  mapfile -t data_files < <(find data -type f -name '*.toml' | sort)
  for name in "${names[@]}"; do binaries+=("build/$name/kindling"); done
  fp="$(fingerprint "${binaries[@]}" "${data_files[@]}" tools/samebits.py tools/check.sh 'audit-digests-v1:one/four:1001:2001')"
  if passed audit-digests "$fp"; then
    echo "   cross-compiler and tie digests unchanged since they passed"
  else
    for name in "${names[@]}"; do
      run=()
      [[ "$name" != sim-a64-* ]] || run=(qemu-aarch64-static)
      for threads in 1 4; do
        "${run[@]}" "build/$name/kindling" proof --threads "$threads" >"$TMP/proof-$name-$threads"
      done
      if [[ "$name" != sim-a64-tie* ]]; then
        "${run[@]}" "build/$name/kindling" learning-gate 1001 1 >"$TMP/learning-$name.json"
        "${run[@]}" "build/$name/kindling" idea-gate 2001 1 >"$TMP/idea-$name.json"
      fi
    done
    python3 tools/samebits.py same "$TMP"/proof-* | sed 's/^/   /'
    python3 - "$TMP" <<'COMPARE'
import json
import re
import sys
from pathlib import Path
root = Path(sys.argv[1])
for kind, keys in [('learning', ['digest']), ('idea', ['sent_digest', 'control_digest', 'missing_digest'])]:
    rows = [json.loads(path.read_text()) for path in sorted(root.glob(kind + '-*.json'))]
    if len(rows) != 3 or any(not re.fullmatch('[0-9a-f]{16}', str(row[key])) for row in rows for key in keys):
        raise SystemExit('M3 ' + kind + ' missing or invalid end digest')
    if any(any(row[key] != rows[0][key] for key in keys) for row in rows):
        raise SystemExit('M3 ' + kind + ' cross-compiler digests differ')
    if kind == 'learning' and any(row['reopen_failures'] for row in rows):
        raise SystemExit('M3 learning phase reopen failed')
    if kind == 'idea' and any(not all(row[key] for key in ['pending_reopen', 'delivered_reopen', 'final_reopen']) for row in rows):
        raise SystemExit('M3 idea phase reopen failed')
print('M3: retained learning/idea end digests match across Clang, GCC and ARM/NDK')
COMPARE
    pass audit-digests "$fp"
  fi
  step "kill, scenes and repeat"
  fp="$(fingerprint build/sim/kindling "${data_files[@]}" tools/killtest.py tools/scenecheck.py tools/check.sh 'audit-recovery-v1')"
  if passed audit-recovery "$fp"; then
    echo "   crash/repeat checks unchanged since they passed"
  else
    python3 tools/killtest.py build/sim/kindling data
    python3 tools/scenecheck.py build/sim/kindling data
    pass audit-recovery "$fp"
  fi
  if [ -n "$thread_pid" ]; then
    wait "$thread_pid" || { tail -60 "$TMP/threads"; echo "Thread checker: FAIL"; return 1; }
    [ "$thread_stamp" = unknown ] || echo "$thread_stamp" >build/sim-tsan/tests.passed
    AUDIT_THREAD_PID=""
    echo "   TSan passed with no race"
  fi
}

if [ "$AUDIT" = 1 ]; then
  audit
  # --deliver still verifies the existing release; it never exports another APK.
  if [ "$DELIVER" = 1 ]; then
    python3 tools/filecheck.py note
    STEP="$(sed -n '1s/^# .*α\([0-9]\{1,2\}\.[0-9]\{1,2\}[a-e]\).*/\1/p' dist/NOTE.md)"
    [ -n "$STEP" ] || { echo "Delivery: note names no alpha"; exit 1; }
    (cd dist && sha256sum --quiet -c kindling.apk.sha256)
    tools/verify-apk.sh dist/kindling.apk release "$STEP"
  fi
  echo "Checks: PASS $COMMIT ($(($(date +%s) - T0)) seconds; audit=1)"
  exit 0
fi

[ -x "$GODOT" ] && command -v gdformat >/dev/null && command -v ruff >/dev/null && command -v ccache >/dev/null \
  && [ -d "$KD_GDUNIT" ] || tools/setup.sh

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
[ ! -f sim/CMakeLists.txt ] || build_one sim sim -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
[ ! -f view/CMakeLists.txt ] || build_one view view -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
echo "   host simulation and extension built"

cpp_tests() {
  echo "== 3 C++ tests"
  [ -f sim/CMakeLists.txt ] || { echo "   no simulation yet"; return; }
  local stamp
  stamp="$(python3 tools/cppcache.py tests build/sim)"
  if [ "$stamp" != unknown ] && [ "$(cat build/sim/tests.passed 2>/dev/null)" = "$stamp" ]; then
    echo "   sim: tests unchanged since they passed"
  else
    quiet ctest --test-dir build/sim --output-on-failure
    [ "$stamp" = unknown ] || echo "$stamp" >build/sim/tests.passed
    echo "   sim: tests passed"
  fi
  for threads in 1 4; do
    build/sim/kindling proof --threads "$threads" >"$TMP/proof-sim-$threads"
  done
  [ ! -f build/view/CTestTestfile.cmake ] || quiet ctest --test-dir build/view --output-on-failure
  echo "   view: tests passed"
  if [ -d data ]; then
    build/sim/kindling catalogue check data >"$TMP/catalogue"
    sed 's/^/   /' "$TMP/catalogue"
  fi
  python3 tools/samebits.py same "$TMP"/proof-* | sed 's/^/   /'
  # the routine lint, on every core, only the files whose code, headers, compile command or rules changed; then the banned
  # list over the simulation's and the extension's own code (A3.4)
  for d in sim view; do
    [ -f "$d/CMakeLists.txt" ] || continue
    mapfile -t SRC < <(printf '%s\n' "${CPP[@]}" | grep -E "^$d/.*\.(cpp|cc)\$" || true)
    LINT="no files to lint"
    if [ "${#SRC[@]}" -gt 0 ]; then
      LINT="$(python3 tools/cppcache.py lint "build/$d" "^$ROOT/$d/" "${SRC[@]}")" || { echo "$LINT"; exit 1; }
    fi
    echo "   $d lint: $LINT"
    mapfile -t SRC < <(printf '%s\n' "${CPP[@]}" | grep -E "^$d/src/.*\.(cpp|cc)\$" || true)
    if [ "${#SRC[@]}" -gt 0 ]; then
      RULES="$(python3 tools/rules.py check "build/$d" "${SRC[@]}")" || { echo "$RULES"; exit 1; }
      echo "   $d ${RULES#Rules: }"
    fi
  done
}

godot_step() {
  echo "== 4 Godot"
  # what the phone's self-check compares with, from the simulation's own build (A2.3)
  [ ! -x build/sim/kindling ] || quiet python3 tools/gamedata.py build/sim/kindling
  PROJECTS=()
  [ ! -f game/project.godot ] || PROJECTS+=(game)
  [ "${#PROJECTS[@]}" -gt 0 ] || echo "   no Godot projects yet"
  for d in "${PROJECTS[@]}"; do
    # what the import, the scripts and the tests read: the project's files, the extensions built for this machine,
    # the game data (build.toml names each catalogue file by its hash) with the scenes' reports, and the tools' versions
    mapfile -t READS < <(printf '%s\n' "${ALL[@]}" | grep "^$d/"; compgen -G "$d/bin/*.so" || true; \
      compgen -G "$d/data/*.toml" || true; compgen -G "$d/data/reports/*" || true)
    # The full scenario benchmark belongs to the explicit audit, not each delivery.
    FP="$(fingerprint "${READS[@]}" tools/godot-scripts.gd "$KD_GODOT_VERSION" "$KD_GDUNIT" "deliver=$DELIVER" "audit=$AUDIT")"
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
          res://addons/gdUnit4/bin/GdUnitCmdTool.gd --ignoreHeadlessMode -a res://test \
          -rd user://gdunit-reports -c \
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
  # Explicitly costly suites use the audit flag; obsolete 3D art suites have been removed.
  KD_CHECK_QUICK=$((1 - AUDIT)) python3 -m unittest discover -s tools/tests >"$TMP/unit" 2>&1 || { cat "$TMP/unit"; exit 1; }
  echo "   $(sed -n 's/^Ran \([0-9]*\) tests.*/\1/p' "$TMP/unit") tool tests passed"
  quiet python3 tools/signing-key.py selftest
  # test_filecheck.py already runs the clean and malformed fixtures.
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

step "7 ID traceability"
python3 tools/filecheck.py ids --merge

if [ "$DELIVER" = 1 ]; then
  step "8 delivery"
  python3 tools/filecheck.py note
  # The step the note delivers, from its title, such as "# Kindling α0.1a: The workshop".
  STEP="$(sed -n '1s/^# .*α\([0-9]\{1,2\}\.[0-9]\{1,2\}[a-e]\).*/\1/p' dist/NOTE.md)"
  [ -n "$STEP" ] || { echo "Delivery: dist/NOTE.md's title names no step, such as 'α0.1a'"; exit 1; }
  # The release build already exported and signed this APK. Avoid exporting it a second time.
  (cd dist && sha256sum --quiet -c kindling.apk.sha256)
  tools/verify-apk.sh dist/kindling.apk release "$STEP"
fi

SECS=$(($(date +%s) - T0))
if [ "$SECS" -lt 120 ]; then TOOK="$SECS seconds"; else TOOK="$(((SECS + 59) / 60)) minutes"; fi
echo "   took $(($(date +%s) - STEP_T)) s"
echo "Checks: PASS $COMMIT ($TOOK; audit=$AUDIT)"
