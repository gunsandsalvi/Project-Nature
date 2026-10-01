#!/bin/bash
# B02 extra slot (F): the guarded wyhash variant "wysafe" (added after the R1 pick, wy,
# turned out to give every being the same draw at the one moment equal to its seed).
# Rebuild + tests, ARM rebuild, wysafe speed, PractRand for wysafe and wy to 2^35.
# Under the CPU lock (about 12 minutes).
set -e
. "$(dirname "$0")/env.sh"
python3 - "$RAW" <<'EOF'
import json, sys
rows = []
for _ in range(3):
    for lang in ('rust', 'cpp'):
        for t in (1, 4):
            rows.append(dict(kernel=f'{lang}:rng', rng='wysafe', threads=t, seconds=1.0, min_reps=3))
            rows.append(dict(kernel=f'{lang}:walk', format='f32', rng='wysafe', threads=t, seconds=1.0, min_reps=3))
rows += [dict(kernel=f'{l}:{k}', format='f32', rng='wysafe', threads=t, seconds=0.1, min_reps=2)
         for l in ('rust', 'cpp', 'cppnc') for k in ('rng', 'walk') for t in (2, 3)]
open(sys.argv[1] + '/wysafe.jsonl', 'w').write(''.join(json.dumps(r) + '\n' for r in rows))
EOF
pr() {  # GEN MODE LOG2 -> results/practrand/GEN-MODE[-2^LOG2].txt
  local out=$B01/results/practrand/$1-$2.txt
  [ "$3" != 34 ] && out=$B01/results/practrand/$1-$2-2^$3.txt
  { echo "# kbench --stream $1 $2 | RNG_test stdin64 -tlmax $3 -multithreaded"; echo "# started $(date -u +%FT%TZ)"
    $KBENCH --stream $1 $2 | $PRACTRAND stdin64 -tlmax $3 -multithreaded; echo "# finished $(date -u +%FT%TZ)"; } > $out 2>&1
  tail -2 $out
}
export -f pr
flock $LOCK bash -c "
  cd $B01/kbench && cargo build --release 2>&1 | grep -E '^(warning|error)|Finished'
  cargo test --release 2>&1 | grep -E 'test result: ok. [1-9]|FAILED|panicked'
  $KBENCH --selftest
  $B01/scripts/build-arm.sh 2>&1 | grep -E 'Finished|error|Java_' | head -4
  qemu-aarch64-static -L /usr/aarch64-linux-gnu $CARGO_TARGET_DIR/aarch64-unknown-linux-gnu/release/kbench --selftest
  $KBENCH --batch $RAW/wysafe.jsonl > $RAW/wysafe.out.jsonl
  for m in moments beings; do pr wysafe \$m 34; done
  for m in moments beings; do pr wy \$m 35; done"
python3 $B01/scripts/bench.py summarize $RAW
