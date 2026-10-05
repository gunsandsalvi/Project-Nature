# The simulation in C++20, with no Godot (A2.1, A2.2): its library, its doctest tests and the kindling tool.
# tools/check.sh builds it five ways (A2.2); view/ builds the library alone into the app's extension.
cmake_minimum_required(VERSION 3.22)
project(kindling_sim LANGUAGES C CXX)

option(KD_SIM_TESTS "Build the tests and the kindling tool" ON)
set(KD_XXHASH "$ENV{KD_XXHASH}" CACHE PATH "xxHash's folder (tools/setup.sh)")
set(KD_DOCTEST "$ENV{KD_DOCTEST}" CACHE PATH "doctest's folder (tools/setup.sh)")

include(cmake/kindling.cmake)
find_package(Threads REQUIRED)

add_library(kd_sim STATIC
    src/kd/core/check.cpp
    src/kd/num/digest.cpp
    src/kd/num/fenv.cpp
    src/kd/proof/proof.cpp
    src/kd/run/workers.cpp)
target_include_directories(kd_sim PUBLIC src)
target_include_directories(kd_sim SYSTEM PRIVATE "${KD_XXHASH}")
target_link_libraries(kd_sim PUBLIC Threads::Threads)
kindling_rules(kd_sim)

if(KD_SIM_TESTS)
    add_executable(kindling tools/kindling.cpp)
    target_link_libraries(kindling PRIVATE kd_sim)
    kindling_rules(kindling)

    add_executable(kd_sim_tests
        tests/main.cpp
        tests/num_test.cpp
        tests/run_test.cpp)
    target_include_directories(kd_sim_tests SYSTEM PRIVATE "${KD_DOCTEST}" "${KD_XXHASH}")
    target_compile_definitions(kd_sim_tests PRIVATE DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS)
    target_link_libraries(kd_sim_tests PRIVATE kd_sim)
    kindling_rules(kd_sim_tests)

    enable_testing()
    add_test(NAME sim COMMAND kd_sim_tests)
endif()
