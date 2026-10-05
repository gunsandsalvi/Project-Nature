# P7 World generation (IMPLEMENTATION α0.5a): the generator's library, its command line, its tests, and, given
# godot-cpp (tools/setup.sh pins it in the cache, KD_GODOT_CPP), its Godot extension for the app; tools/check.sh
# builds and runs them. It uses P5's maths, keyed chance, hashes and arm64 check (prototypes/samebits) and P6's
# thread pool (prototypes/minds/src/pool), written once. Compiled as the simulation will be (A2.2, A3.4): C++20,
# warnings as errors, no fast-math and no contraction into fused multiply-adds. Pre-production code (research 00).
cmake_minimum_required(VERSION 3.22)
project(worldgen CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SAMEBITS "${CMAKE_CURRENT_SOURCE_DIR}/../samebits/src")
set(MINDS "${CMAKE_CURRENT_SOURCE_DIR}/../minds/src")
set(SOURCES src/noise.cpp src/fft.cpp src/land.cpp src/climate.cpp src/life.cpp src/choose.cpp src/settle.cpp
    src/maps.cpp src/area.cpp src/ecology.cpp "${SAMEBITS}/maths.cpp" "${MINDS}/pool.cpp")
add_library(worldgen STATIC ${SOURCES})
target_include_directories(worldgen PUBLIC src "${SAMEBITS}" "${MINDS}")
target_compile_options(worldgen PUBLIC -Wall -Wextra -Werror -ffp-contract=off -fno-fast-math)
find_package(Threads REQUIRED)
target_link_libraries(worldgen PUBLIC Threads::Threads)

add_executable(worldgen_cli src/cli.cpp)
target_link_libraries(worldgen_cli PRIVATE worldgen)
# P9 Ecology: worlds run headless for 100 years with nobody in them (IMPLEMENTATION α0.6a)
add_executable(ecology_cli src/ecology_cli.cpp)
target_link_libraries(ecology_cli PRIVATE worldgen)

enable_testing()
# doctest's one header, pinned by tools/setup.sh in the cache
set(DOCTEST_DIR "$ENV{KD_DOCTEST}" CACHE PATH "the folder holding doctest.h")
add_executable(worldgen_test tests/worldgen_test.cpp)
target_include_directories(worldgen_test SYSTEM PRIVATE "${DOCTEST_DIR}")
target_link_libraries(worldgen_test PRIVATE worldgen)
add_test(NAME worldgen_test COMMAND worldgen_test)
# the same small run built for arm64 and run under qemu, against this machine's, with P5's script (RES-05)
set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")
list(TRANSFORM SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/" REGEX "^src/")
add_test(NAME arm64_same_bits COMMAND "${SAMEBITS}/../arm64.sh" $<TARGET_FILE:worldgen_cli> small "-I${SRC}"
    "-I${SAMEBITS}" "-I${MINDS}" ${SOURCES} "${SRC}/cli.cpp")

if(NOT GODOT_CPP AND EXISTS "$ENV{KD_GODOT_CPP}/CMakeLists.txt")
    set(GODOT_CPP "$ENV{KD_GODOT_CPP}")
endif()
if(GODOT_CPP)
    add_subdirectory(extension)
endif()
