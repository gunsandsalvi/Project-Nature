# P5 The same bits (IMPLEMENTATION α0.4a): the toy world's library, its command line, its tests, and, given godot-cpp
# (tools/setup.sh pins it in the cache, KD_GODOT_CPP), its Godot extension for the app; tools/check.sh builds and runs
# them. Compiled as the simulation will be (A2.2, A3.4): C++20, warnings as errors, no fast-math and no
# contraction into fused multiply-adds. Pre-production code (research 00).
cmake_minimum_required(VERSION 3.22)
project(samebits CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(samebits STATIC src/maths.cpp src/world.cpp)
target_include_directories(samebits PUBLIC src)
target_compile_options(samebits PUBLIC -Wall -Wextra -Werror -ffp-contract=off -fno-fast-math)
find_package(Threads REQUIRED)
target_link_libraries(samebits PUBLIC Threads::Threads)

add_executable(samebits_cli src/cli.cpp)
target_link_libraries(samebits_cli PRIVATE samebits)

enable_testing()
# doctest's one header, pinned by tools/setup.sh in the cache
set(DOCTEST_DIR "$ENV{KD_DOCTEST}" CACHE PATH "the folder holding doctest.h")
add_executable(samebits_test tests/samebits_test.cpp)
target_include_directories(samebits_test SYSTEM PRIVATE "${DOCTEST_DIR}")
target_link_libraries(samebits_test PRIVATE samebits)
add_test(NAME samebits_test COMMAND samebits_test)
# the same run built for arm64 and run under qemu, against this machine's (RES-05)
set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")
add_test(NAME arm64_same_bits COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/arm64.sh" $<TARGET_FILE:samebits_cli> 30 "-I${SRC}"
    "${SRC}/maths.cpp" "${SRC}/world.cpp" "${SRC}/cli.cpp")

if(NOT GODOT_CPP AND EXISTS "$ENV{KD_GODOT_CPP}/CMakeLists.txt")
    set(GODOT_CPP "$ENV{KD_GODOT_CPP}")
endif()
if(GODOT_CPP)
    add_subdirectory(extension)
endif()
