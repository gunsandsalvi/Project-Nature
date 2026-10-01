#!/bin/sh
# B80 / T6 part A: one 10-minute half of the sustained all-core run. Run it under the CPU lock:
#   flock <lock> sh parta.sh half1      (later: half2)
# Writes results/parta-<half>.csv (units of work per 10 s, CPU shares incl. steal from /proc/stat)
# and results/parta-<half>-vmstat.txt (vmstat every 60 s; its "st" column is steal).
HERE=$(cd "$(dirname "$0")" && pwd)
CACHE=${CACHE:?set CACHE to the shared cache folder}
BIN=${B80_BIN:-$CACHE/b80-target/release/b80}
HALF=$1
mkdir -p "$HERE/results"
date -u +"start %Y-%m-%dT%H:%M:%SZ" > "$HERE/results/parta-$HALF-vmstat.txt"
vmstat -n -t 60 11 >> "$HERE/results/parta-$HALF-vmstat.txt" &
VM=$!
"$BIN" burn --secs 600 --threads 4 --tick 10 --csv "$HERE/results/parta-$HALF.csv" > /dev/null
wait $VM
date -u +"end %Y-%m-%dT%H:%M:%SZ" >> "$HERE/results/parta-$HALF-vmstat.txt"
