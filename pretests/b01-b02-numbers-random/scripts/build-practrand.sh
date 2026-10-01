#!/bin/bash
# B02: builds PractRand pre0.95 (source from SourceForge) into the shared cache.
# Run under the CPU lock: flock $LOCK scripts/build-practrand.sh
set -e
. "$(dirname "$0")/env.sh"
P=$CACHE/practrand
mkdir -p $P
if [ ! -d $P/src/src ]; then
  curl -sSfL -o $P/PractRand-pre0.95.zip "https://sourceforge.net/projects/pracrand/files/PractRand-pre0.95.zip/download"
  unzip -q -o $P/PractRand-pre0.95.zip -d $P/src
fi
cd $P/src
g++ -O3 -march=native -std=c++14 -pthread -w -Iinclude src/*.cpp src/RNGs/*.cpp src/RNGs/other/*.cpp \
  tools/RNG_test.cpp -o RNG_test
ls -la RNG_test
