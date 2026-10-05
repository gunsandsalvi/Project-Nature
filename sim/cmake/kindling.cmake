# The rules for all of Kindling's own C++ (A2.2, A3.4): every target that includes the simulation's headers, in
# sim/, view/ and their tests, is built with these, since an inline function compiled differently in one of them
# can replace the simulation's own copy at link time (research 18).
#
# -ffp-contract=off must be the last floating-point flag on each compile line: a later -ffp-model turns contraction
# into fused multiply-adds back on without a warning. Plain char is unsigned on every build, as on arm64, so no
# arithmetic on it can differ between chips. tools/samebits.py checks both on the compile commands.

function(kindling_rules target)
    target_compile_features(${target} PUBLIC cxx_std_20)
    set_target_properties(${target} PROPERTIES
        CXX_EXTENSIONS OFF
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        POSITION_INDEPENDENT_CODE ON)
    target_compile_options(${target} PRIVATE
        -Wall -Wextra -Werror -Wdouble-promotion -Wfloat-conversion
        -fno-exceptions -funsigned-char
        -ffunction-sections -fdata-sections
        $<$<CXX_COMPILER_ID:GNU>:-fexcess-precision=standard>
        -fno-fast-math -fno-math-errno
        -ffp-contract=off)
endfunction()

# The same for vendored C code (CORE-MATH, zstd): its floating-point flags, without our warnings.
function(kindling_c_rules target)
    set_target_properties(${target} PROPERTIES
        C_VISIBILITY_PRESET hidden
        POSITION_INDEPENDENT_CODE ON)
    target_compile_options(${target} PRIVATE
        -funsigned-char -ffunction-sections -fdata-sections
        $<$<C_COMPILER_ID:GNU>:-fexcess-precision=standard>
        -fno-fast-math -fno-math-errno
        -ffp-contract=off)
endfunction()
