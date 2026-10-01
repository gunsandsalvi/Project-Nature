#!/bin/bash
# B01: three more interleaved rounds of the speed sweep and the Java timings (slot E),
# added because single 1 s runs on this shared VM vary by +-10-30%. Under the CPU lock.
set -e
. "$(dirname "$0")/env.sh"
RAW=${RAW:-/tmp/claude-0/-home-user-Project-Nature/d9fdddff-7118-505f-be5c-63935305a20b/scratchpad/b01/raw}
JCLS=$RAW/jclasses
flock $LOCK bash -c "
  $KBENCH --batch $RAW/sweep.jsonl > $RAW/sweep2.out.jsonl
  mkdir -p $JCLS && javac -d $JCLS $B01/jvm/Kernels.java
  java -cp $JCLS dev.kindling.pretests.Kernels --batch $RAW/java.jsonl > $RAW/java2.out.jsonl 2>/dev/null"
python3 $B01/scripts/bench.py summarize $RAW
