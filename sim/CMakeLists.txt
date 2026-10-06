# The simulation in C++20, with no Godot (A2.1, A2.2): its library, its doctest tests and the kindling tool.
# tools/check.sh builds it five ways (A2.2); view/ builds the library alone into the app's extension.
cmake_minimum_required(VERSION 3.22)
project(kindling_sim LANGUAGES C CXX)

option(KD_SIM_TESTS "Build the tests and the kindling tool" ON)
set(KD_XXHASH "$ENV{KD_XXHASH}" CACHE PATH "xxHash's folder (tools/setup.sh)")
set(KD_DOCTEST "$ENV{KD_DOCTEST}" CACHE PATH "doctest's folder (tools/setup.sh)")
set(KD_TOMLPP "$ENV{KD_TOMLPP}" CACHE PATH "toml++'s folder (tools/setup.sh)")
set(KD_ENTT "$ENV{KD_ENTT}" CACHE PATH "EnTT's folder (tools/setup.sh)")
set(KD_ZSTD "$ENV{KD_ZSTD}" CACHE PATH "zstd's folder (tools/setup.sh)")

include(cmake/kindling.cmake)
find_package(Threads REQUIRED)

# CORE-MATH's correctly rounded maths (A3.4), vendored at a pinned commit by tools/core-math.py: C, with our
# floating-point flags and without our warnings.
file(GLOB KD_CORE_MATH CONFIGURE_DEPENDS thirdparty/core-math/src/binary64/*/*.c)
add_library(kd_core_math STATIC ${KD_CORE_MATH})
kindling_c_rules(kd_core_math)

# zstd 1.5.7's library, which compresses the saves' snapshots (A3.7), from tools/setup.sh: C, with our floating-point
# flags and without our warnings; no assembly, so every build compresses with the same C.
file(GLOB KD_ZSTD_SOURCES CONFIGURE_DEPENDS "${KD_ZSTD}/lib/common/*.c" "${KD_ZSTD}/lib/compress/*.c"
    "${KD_ZSTD}/lib/decompress/*.c")
add_library(kd_zstd STATIC ${KD_ZSTD_SOURCES})
target_compile_definitions(kd_zstd PRIVATE ZSTD_DISABLE_ASM=1 ZSTD_LEGACY_SUPPORT=0)
target_include_directories(kd_zstd SYSTEM PUBLIC "${KD_ZSTD}/lib")
kindling_c_rules(kd_zstd)

# NVIDIA's FLIP (A4.8), vendored at a pinned commit with the one call the look makes into it: float maths with the
# platform's functions, kept out of the simulation's own code; our flags, and its header a system one, which our
# warnings skip.
add_library(kd_flip STATIC thirdparty/flip/kd_flip.cpp)
target_include_directories(kd_flip SYSTEM PUBLIC thirdparty/flip)
kindling_rules(kd_flip)

add_library(kd_sim STATIC
    src/kd/bench/code.cpp
    src/kd/bench/scenarios.cpp
    src/kd/chance/chance.cpp
    src/kd/core/check.cpp
    src/kd/data/catalogue.cpp
    src/kd/data/checks.cpp
    src/kd/data/folder.cpp
    src/kd/data/kinds.cpp
    src/kd/data/loader.cpp
    src/kd/data/orders.cpp
    src/kd/data/toml.cpp
    src/kd/data/units.cpp
    src/kd/data/walkers.cpp
    src/kd/event/queue.cpp
    src/kd/look/card.cpp
    src/kd/look/colour.cpp
    src/kd/look/frame.cpp
    src/kd/look/measures.cpp
    src/kd/demo/checks.cpp
    src/kd/demo/clockwork.cpp
    src/kd/demo/crowd_scene.cpp
    src/kd/demo/crowd_world.cpp
    src/kd/demo/kept.cpp
    src/kd/num/angle.cpp
    src/kd/num/convert.cpp
    src/kd/num/digest.cpp
    src/kd/num/fenv.cpp
    src/kd/num/maths.cpp
    src/kd/proof/fixture.cpp
    src/kd/proof/maths_cases.cpp
    src/kd/proof/proof.cpp
    src/kd/run/marked.cpp
    src/kd/run/runner.cpp
    src/kd/run/thread.cpp
    src/kd/run/workers.cpp
    src/kd/save/archive.cpp
    src/kd/save/files.cpp
    src/kd/save/keeper.cpp
    src/kd/save/log.cpp
    src/kd/save/snapshot.cpp
    src/kd/save/versions.cpp
    src/kd/scene/report.cpp
    src/kd/scene/scene.cpp
    src/kd/time/calendar.cpp
    src/kd/time/duration.cpp
    src/kd/world/parts.cpp
    src/kd/world/upgrades.cpp
    src/kd/world/world.cpp)
target_include_directories(kd_sim PUBLIC src)
target_include_directories(kd_sim SYSTEM PUBLIC "${KD_ENTT}")
target_include_directories(kd_sim SYSTEM PRIVATE "${KD_XXHASH}" "${KD_TOMLPP}" thirdparty/core-math)
target_link_libraries(kd_sim PUBLIC Threads::Threads PRIVATE kd_core_math kd_zstd kd_flip)
kindling_rules(kd_sim)
# test switches exist in the cloud's test builds only, never in the game's own (RES-10, PRN-12)
if(KD_SIM_TESTS)
    target_compile_definitions(kd_sim PUBLIC KD_TEST_SWITCHES)
endif()

if(KD_SIM_TESTS)
    add_executable(kindling tools/kindling.cpp)
    target_link_libraries(kindling PRIVATE kd_sim)
    kindling_rules(kindling)

    add_executable(kd_sim_tests
        tests/bench_test.cpp
        tests/calendar_test.cpp
        tests/catalogue_test.cpp
        tests/chance_test.cpp
        tests/checks_test.cpp
        tests/data_test.cpp
        tests/duration_test.cpp
        tests/look_card_test.cpp
        tests/look_frame_test.cpp
        tests/look_test.cpp
        tests/main.cpp
        tests/maths_test.cpp
        tests/num_test.cpp
        tests/numbers_test.cpp
        tests/run_test.cpp
        tests/runner_test.cpp
        tests/save_test.cpp
        tests/scene_test.cpp
        tests/world_test.cpp)
    target_include_directories(kd_sim_tests SYSTEM PRIVATE "${KD_DOCTEST}" "${KD_XXHASH}")
    # the repository, where the tests find the data folder and the corpus of old worlds (PLT-09)
    target_compile_definitions(kd_sim_tests PRIVATE DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS
                                                    KD_REPO="${CMAKE_CURRENT_SOURCE_DIR}/..")
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
