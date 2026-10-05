# P14 Sound (IMPLEMENTATION α0.7c): the sound library, its command line, its tests, and, given godot-cpp
# (tools/setup.sh pins it in the cache, KD_GODOT_CPP), its Godot extension for the app; tools/check.sh builds and
# runs them. Pre-production code (research 00).
cmake_minimum_required(VERSION 3.22)
project(sound CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(sound STATIC src/sound.cpp)
target_include_directories(sound PUBLIC src)
target_compile_options(sound PUBLIC -Wall -Wextra -Werror)

# writes every base sound, talk in each voice and feeling, and the cries as WAV files, for listening
add_executable(sound_cli src/cli.cpp)
target_link_libraries(sound_cli PRIVATE sound)

enable_testing()
# doctest's one header, pinned by tools/setup.sh in the cache
set(DOCTEST_DIR "$ENV{KD_DOCTEST}" CACHE PATH "the folder holding doctest.h")
add_executable(sound_test tests/sound_test.cpp)
target_include_directories(sound_test SYSTEM PRIVATE "${DOCTEST_DIR}")
target_link_libraries(sound_test PRIVATE sound)
# the murmur's bank, as bank.py renders it and the app ships it
target_compile_definitions(sound_test PRIVATE KD_BANK="${CMAKE_CURRENT_SOURCE_DIR}/../app/sound/bank")
add_test(NAME sound_test COMMAND sound_test)

if(NOT GODOT_CPP AND EXISTS "$ENV{KD_GODOT_CPP}/CMakeLists.txt")
    set(GODOT_CPP "$ENV{KD_GODOT_CPP}")
endif()
if(GODOT_CPP)
    add_subdirectory(extension)
endif()
