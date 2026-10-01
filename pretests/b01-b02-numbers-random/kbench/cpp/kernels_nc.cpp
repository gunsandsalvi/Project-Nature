// B01: the same C++ kernels with exported names prefixed cppnc_; build.rs compiles this
// file with -ffp-contract=off (no fused multiply-add), for the cross-machine checks.
#define KB_PREFIX cppnc_
#include "kernels.cpp"
