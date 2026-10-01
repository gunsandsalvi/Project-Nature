// B78/B80: the same core, run headless on Linux (the cloud side of PLT-05).
#include <cstdio>
#include "kcore.h"
int main() { std::printf("mix(42, 1000000) = %016llx\n", (unsigned long long)kcore_mix(42, 1000000)); }
