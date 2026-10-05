# The simulation in C++20, with no Godot (A2.1, A2.2): its library, its doctest tests and the kindling tool.
# tools/check.sh builds it five ways (A2.2); view/ builds the library alone into the app's extension.
cmake_minimum_required(VERSION 3.22)
project(kindling_sim LANGUAGES C CXX)

option(KD_SIM_TESTS "Build the tests and the kindling tool" ON)
set(KD_XXHASH "$ENV{KD_XXHASH}" CACHE PATH "xxHash's folder (tools/setup.sh)")
set(KD_DOCTEST "$ENV{KD_DOCTEST}" CACHE PATH "doctest's folder (tools/setup.sh)")

include(cmake/kindling.cmake)
find_package(Threads REQUIRED)

# CORE-MATH's correctly rounded maths (A3.4), vendored at a pinned commit by tools/core-math.py: C, with our
# floating-point flags and without our warnings.
file(GLOB KD_CORE_MATH CONFIGURE_DEPENDS thirdparty/core-math/src/binary64/*/*.c)
add_library(kd_core_math STATIC ${KD_CORE_MATH})
kindling_c_rules(kd_core_math)

add_library(kd_sim STATIC
    src/kd/chance/chance.cpp
    src/kd/core/check.cpp
    src/kd/num/angle.cpp
    src/kd/num/convert.cpp
    src/kd/num/digest.cpp
    src/kd/num/fenv.cpp
    src/kd/num/maths.cpp
    src/kd/proof/maths_cases.cpp
    src/kd/proof/proof.cpp
    src/kd/run/workers.cpp
    src/kd/time/calendar.cpp)
target_include_directories(kd_sim PUBLIC src)
target_include_directories(kd_sim SYSTEM PRIVATE "${KD_XXHASH}" thirdparty/core-math)
target_link_libraries(kd_sim PUBLIC Threads::Threads PRIVATE kd_core_math)
kindling_rules(kd_sim)

if(KD_SIM_TESTS)
    add_executable(kindling tools/kindling.cpp)
    target_link_libraries(kindling PRIVATE kd_sim)
    kindling_rules(kindling)

    add_executable(kd_sim_tests
        tests/calendar_test.cpp
        tests/chance_test.cpp
        tests/main.cpp
        tests/maths_test.cpp
        tests/num_test.cpp
        tests/numbers_test.cpp
        tests/run_test.cpp)
    target_include_directories(kd_sim_tests SYSTEM PRIVATE "${KD_DOCTEST}" "${KD_XXHASH}")
    target_compile_definitions(kd_sim_tests PRIVATE DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS)
    target_link_libraries(kd_sim_tests PRIVATE kd_sim)
    kindling_rules(kd_sim_tests)

    enable_testing()
    add_test(NAME sim COMMAND kd_sim_tests)

    # The maths oracle, where MPFR is: the cloud's own builds, never the cross builds for arm64.
    find_path(KD_MPFR_INCLUDE mpfr.h)
    find_library(KD_MPFR mpfr)
    find_library(KD_GMP gmp)
    if(KD_MPFR_INCLUDE AND KD_MPFR AND KD_GMP AND NOT CMAKE_CROSSCOMPILING)
        add_executable(kd_oracle tests/oracle.cpp)
        target_include_directories(kd_oracle SYSTEM PRIVATE "${KD_MPFR_INCLUDE}")
        target_link_libraries(kd_oracle PRIVATE kd_sim "${KD_MPFR}" "${KD_GMP}")
        kindling_rules(kd_oracle)
        add_test(NAME oracle COMMAND kd_oracle)
    endif()
endif()
