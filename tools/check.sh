#!/usr/bin/env bash
# The checks before work joins main (PRC-10, A17), in order, stopping at the first failure:
#   1 formats     GDScript (gdformat), C++ (clang-format 18) and Python (ruff)
#   2 lints       GDScript (gdlint), Python (ruff) and shell (bash -n); C++'s with its build, in 3
#   3 C++         the five builds (A2.2), through ccache, the phone compiler's twice more with libc++'s order of
#                 ties randomized under two seeds, and the simulation's tests with GCC's thread checker; then, beside
#                 steps 4 and 5, the simulation's doctest tests on its four builds and under the thread checker, the
#                 kill test (tools/killtest.py), the scenes and the repeat check
#                 (tools/scenecheck.py), the same-bits check (every proof suite one digest on x86-64 with clang and
#                 GCC, on arm64 with GCC and the phone's own compiler, and on the randomized builds, on one thread and
#                 four), the scans of the flags and the built code (tools/samebits.py), the banned list
#                 (tools/rules.py) and the code linted (clang-tidy 18); tests, rules and lint only when what they
#                 depend on changed since they passed (tools/cppcache.py)
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
# The five builds of our C++ (A2.2): the simulation with clang (its tests and tool), with GCC and its
# undefined-behaviour checks, and for arm64 with GCC and with the phone's own compiler (NDK r30) as static
# programs run under qemu, which the same-bits check compares; and the extension with clang, which the Godot tests
# load. Each through ccache.
SANITIZE="-fsanitize=undefined -fsanitize=float-cast-overflow -fno-sanitize-recover=all"
SIM_BUILDS=(sim sim-gcc sim-a64-gcc sim-a64-ndk)
TIE_BUILDS=(sim-a64-tie1 sim-a64-tie2)
build_one() {  # name, source folder, configure options...; TARGET, if set, builds only that target
  local name="$1" src="$2"
  shift 2
  quiet cmake -S "$src" -B "build/$name" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache "$@"
  quiet cmake --build "build/$name" ${TARGET:+--target "$TARGET"}
}
if [ -f sim/CMakeLists.txt ]; then
  build_one sim sim -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
  build_one sim-gcc sim -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ "-DCMAKE_C_FLAGS=$SANITIZE" \
    "-DCMAKE_CXX_FLAGS=$SANITIZE" "-DCMAKE_EXE_LINKER_FLAGS=$SANITIZE"
  build_one sim-a64-gcc sim "-DCMAKE_TOOLCHAIN_FILE=$ROOT/sim/cmake/a64-gcc.cmake"
  NDK=("-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" -DANDROID_ABI=arm64-v8a
    -DANDROID_PLATFORM=android-24 -DANDROID_STL=c++_static -DCMAKE_EXE_LINKER_FLAGS=-static
    -DCMAKE_CROSSCOMPILING_EMULATOR=qemu-aarch64-static)
  build_one sim-a64-ndk sim "${NDK[@]}"
  # the tests once more under GCC's thread checker, since a race can damage memory without failing a test (A2.2)
  build_one sim-tsan sim -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_FLAGS=-fsanitize=thread \
    -DCMAKE_CXX_FLAGS=-fsanitize=thread -DCMAKE_EXE_LINKER_FLAGS=-fsanitize=thread
  # libc++ shuffles before each sort that may leave ties in any order, so a result that depends on ties moves
  for seed in 1 2; do
    TARGET=kindling build_one "sim-a64-tie$seed" sim "${NDK[@]}" "-DCMAKE_CXX_FLAGS=-D_LIBCPP_DEBUG_RANDOMIZE_UNSPECIFIED_STABILITY \
-D_LIBCPP_DEBUG_RANDOMIZE_UNSPECIFIED_STABILITY_SEED=$seed"
  done
fi
[ ! -f view/CMakeLists.txt ] || build_one view view -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
echo "   the simulation's four builds, its two with ties randomized, its thread-checked one, and the extension built"

# The simulation's doctest tests on its four builds, the same-bits check across them, the scans of what the
# compilers did, and the code linted (clang-tidy 18).
cpp_tests() {
  echo "== 3 C++ tests"
  [ -f sim/CMakeLists.txt ] || { echo "   no simulation yet"; return; }
  # the thread checker over every test, on a core of its own beside the rest, and only when they changed since it
  # passed; it stops at the first race
  THREADS_STAMP="$(python3 tools/cppcache.py tests build/sim-tsan)"
  THREADS_RESULT="tests unchanged since they passed"
  if [ "$THREADS_STAMP" = unknown ] || [ "$(cat build/sim-tsan/tests.passed 2>/dev/null)" != "$THREADS_STAMP" ]; then
    TSAN_OPTIONS="halt_on_error=1" build/sim-tsan/kd_sim_tests >"$TMP/threads" 2>&1 &
    THREADS=$!
    THREADS_RESULT="tests passed with no race"
  fi
  for b in "${SIM_BUILDS[@]}"; do
    B="build/$b"
    # the tests run again only when something they are built from changed since they passed (tools/cppcache.py)
    STAMP="$(python3 tools/cppcache.py tests "$B")"
    if [ "$STAMP" != unknown ] && [ "$(cat "$B/tests.passed" 2>/dev/null)" = "$STAMP" ]; then
      TESTS="tests unchanged since they passed"
    else
      quiet ctest --test-dir "$B" --output-on-failure
      [ "$STAMP" = unknown ] || echo "$STAMP" >"$B/tests.passed"
      TESTS="tests passed"
    fi
    # every proof suite on one thread and four, on every build; all must give one digest (A3.4)
    RUN=()
    [[ "$b" != sim-a64-* ]] || RUN=(qemu-aarch64-static)
    for threads in 1 4; do
      "${RUN[@]}" "$B/kindling" proof --threads "$threads" >"$TMP/proof-$b-$threads" || { cat "$TMP/proof-$b-$threads"; exit 1; }
    done
    echo "   $b: $TESTS"
  done
  if [ -n "${THREADS:-}" ]; then
    wait "$THREADS" || { grep -vE '^\s*$' "$TMP/threads" | tail -60; echo "Thread checker: FAIL"; exit 1; }
    [ "$THREADS_STAMP" = unknown ] || echo "$THREADS_STAMP" >build/sim-tsan/tests.passed
  fi
  echo "   sim-tsan: $THREADS_RESULT"
  # the extension's own tests, of what needs no Godot (the speed loop)
  if [ -f build/view/CTestTestfile.cmake ]; then
    quiet ctest --test-dir build/view --output-on-failure
    echo "   view: tests passed"
  fi
  for b in "${TIE_BUILDS[@]}"; do
    for threads in 1 4; do
      qemu-aarch64-static "build/$b/kindling" proof --threads "$threads" >"$TMP/proof-$b-$threads" \
        || { cat "$TMP/proof-$b-$threads"; exit 1; }
    done
  done
  # the catalogues, loaded and checked by the simulation's own tool, every problem named by file, line and column
  if [ -d data ] && [ -x build/sim/kindling ]; then
    build/sim/kindling catalogue check data >"$TMP/catalogue" || { cat "$TMP/catalogue"; exit 1; }
    sed 's/^/   /' "$TMP/catalogue"
    # the kill test: a kept world killed at 100 moments ends as an unbroken one (PLT-07, A3.7)
    python3 tools/killtest.py build/sim/kindling data >"$TMP/kill" || { cat "$TMP/kill"; exit 1; }
    sed 's/^/   /' "$TMP/kill"
    # the scenes: each passes, the planted one flags every oddity, a failed rule is judged on twice its runs, and the
    # repeat check runs one scene and the benchmark world on one core and on four with a stop between (PRC-10)
    python3 tools/scenecheck.py build/sim/kindling data >"$TMP/scenes" || { cat "$TMP/scenes"; exit 1; }
    sed 's/^/   /' "$TMP/scenes"
  fi
  python3 tools/samebits.py same "$TMP"/proof-* | sed 's/^/   /'
  [ "${PIPESTATUS[0]}" -eq 0 ] || exit 1
  python3 tools/samebits.py flags "${SIM_BUILDS[@]/#/build/}" build/view | sed 's/^/   /'
  [ "${PIPESTATUS[0]}" -eq 0 ] || exit 1
  python3 tools/samebits.py scan "${SIM_BUILDS[@]/#/build/}" build/view | sed 's/^/   /'
  [ "${PIPESTATUS[0]}" -eq 0 ] || exit 1
  # the lint, on every core, only the files whose code, headers, compile command or rules changed; then the banned
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
