#!/bin/bash
# B01/B02: all cloud measurements, each slot under the shared CPU lock (< 15 min each).
# Usage: scripts/run-all.sh [slotA slotB slotC slotD]   (default: all four)
# Raw output goes to $RAW (scratchpad); summaries go to results/.
set -e
. "$(dirname "$0")/env.sh"
ARM_KBENCH=$CARGO_TARGET_DIR/aarch64-unknown-linux-gnu/release/kbench
mkdir -p $RAW
python3 $B01/scripts/bench.py gen $RAW
JCLS=$RAW/jclasses

slotA() {  # speed sweep (3 interleaved rounds), 2- and 3-thread checksums, Java
  flock $LOCK bash -c "
    $KBENCH --batch $RAW/sweep.jsonl > $RAW/sweep.out.jsonl
    $KBENCH --batch $RAW/threads.jsonl > $RAW/threads.out.jsonl
    mkdir -p $JCLS && javac -d $JCLS $B01/jvm/Kernels.java
    java -cp $JCLS dev.kindling.pretests.Kernels --batch $RAW/java.jsonl > $RAW/java.out.jsonl 2>/dev/null"
}
slotB() {  # ARM checksums under qemu, then PractRand for splitmix and philox
  flock $LOCK bash -c "
    qemu-aarch64-static -L /usr/aarch64-linux-gnu $ARM_KBENCH --selftest > $RAW/qemu-selftest.json
    qemu-aarch64-static -L /usr/aarch64-linux-gnu $ARM_KBENCH --batch $RAW/qemu.jsonl > $RAW/qemu.out.jsonl
    for g in splitmix philox; do for m in moments beings; do $B01/scripts/practrand.sh \$g \$m 34; done; done"
}
slotC() {
  flock $LOCK bash -c "for g in squares pcg wy; do for m in moments beings; do $B01/scripts/practrand.sh \$g \$m 34; done; done"
}
slotD() {  # ChaCha8 (slow generator), then the fortune-retry streams at 2^32
  flock $LOCK bash -c "
    for m in moments beings; do $B01/scripts/practrand.sh chacha8 \$m 34; done
    for g in splitmix philox squares pcg wy chacha8 wysafe; do $B01/scripts/practrand.sh \$g retry 32; done"
}
slotE() { $B01/scripts/run-extra.sh; }   # 3 more speed rounds (single runs vary +-10-30% here)
slotF() { $B01/scripts/run-wysafe.sh; }  # guarded wy variant: speed, PractRand; wy to 2^35
for s in ${@:-slotA slotB slotC slotD slotE slotF}; do
  echo "== $s start $(date -u +%T)"; $s; echo "== $s done $(date -u +%T)"
done
python3 $B01/scripts/bench.py summarize $RAW
