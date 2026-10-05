# P6 A thousand minds (IMPLEMENTATION α0.4b): the minds' library, its command line, its tests, and, given godot-cpp
# (tools/setup.sh pins it in the cache, KD_GODOT_CPP), its Godot extension for the app; tools/check.sh builds and
# runs them. It uses P5's own maths, keyed chance and hashes (prototypes/samebits/src), written once. Compiled as the
# simulation will be (A2.2, A3.4): C++20, warnings as errors, no fast-math and no contraction into fused multiply-adds.
# Pre-production code (research 00).
cmake_minimum_required(VERSION 3.22)
project(minds CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SAMEBITS "${CMAKE_CURRENT_SOURCE_DIR}/../samebits/src")
add_library(minds STATIC src/land.cpp src/paths.cpp src/pool.cpp src/minds.cpp "${SAMEBITS}/maths.cpp")
target_include_directories(minds PUBLIC src "${SAMEBITS}")
target_compile_options(minds PUBLIC -Wall -Wextra -Werror -ffp-contract=off -fno-fast-math)
find_package(Threads REQUIRED)
target_link_libraries(minds PUBLIC Threads::Threads)

add_executable(minds_cli src/cli.cpp)
target_link_libraries(minds_cli PRIVATE minds)

enable_testing()
# doctest's one header, pinned by tools/setup.sh in the cache
set(DOCTEST_DIR "$ENV{KD_DOCTEST}" CACHE PATH "the folder holding doctest.h")
add_executable(minds_test tests/minds_test.cpp)
target_include_directories(minds_test SYSTEM PRIVATE "${DOCTEST_DIR}")
target_link_libraries(minds_test PRIVATE minds)
add_test(NAME minds_test COMMAND minds_test)
# the same run built for arm64 and run under qemu, against this machine's, with P5's script (RES-05)
set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")
add_test(NAME arm64_same_bits COMMAND "${SAMEBITS}/../arm64.sh" $<TARGET_FILE:minds_cli> 2 "-I${SRC}" "-I${SAMEBITS}"
    "${SRC}/land.cpp" "${SRC}/paths.cpp" "${SRC}/pool.cpp" "${SRC}/minds.cpp" "${SRC}/cli.cpp" "${SAMEBITS}/maths.cpp")

if(NOT GODOT_CPP AND EXISTS "$ENV{KD_GODOT_CPP}/CMakeLists.txt")
    set(GODOT_CPP "$ENV{KD_GODOT_CPP}")
endif()
if(GODOT_CPP)
    add_subdirectory(extension)
endif()
