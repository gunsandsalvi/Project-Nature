#!/bin/bash
# B02: one PractRand run on one keyed stream. Usage: scripts/practrand.sh GEN MODE LOG2BYTES
#   MODE: moments (one key, moments 0,1,2...) | beings (beings 0,1,2... at one moment)
#         | retry (each chance event's first draw, then its fortune retry, alternating)
# Run under the CPU lock. Output: results/practrand/GEN-MODE.txt
set -e
. "$(dirname "$0")/env.sh"
GEN=$1; MODE=$2; LEN=${3:-34}
mkdir -p $B01/results/practrand
OUT=$B01/results/practrand/$GEN-$MODE.txt
{
  echo "# kbench --stream $GEN $MODE | RNG_test stdin64 -tlmax $LEN -multithreaded"
  echo "# started $(date -u +%FT%TZ)"
  $KBENCH --stream $GEN $MODE | $PRACTRAND stdin64 -tlmax $LEN -multithreaded
  echo "# finished $(date -u +%FT%TZ)"
} > $OUT 2>&1
tail -3 $OUT
