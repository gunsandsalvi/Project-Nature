# P5's Godot extension (IMPLEMENTATION α0.4a): the toy world's sources and godot-cpp, pinned at its 4.5 release, which
# Godot 4.7 loads, built into one shared library: natively for the cloud's tests, and for the phone's arm64 Android by
# tools/build.sh with the NDK. Each build puts its library where the app's samebits.gdextension looks for it, so
# nothing built is committed. Compiled as the simulation is (A2.2, A3.4): no fast-math, no contraction.
# Pre-production code (research 00).
cmake_minimum_required(VERSION 3.22)
project(samebits_gd CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GODOTCPP_BUILD_PROFILE "${CMAKE_CURRENT_SOURCE_DIR}/build_profile.json" CACHE PATH "" FORCE)
if(NOT GODOTCPP_TARGET)
    set(GODOTCPP_TARGET template_release CACHE STRING "" FORCE)
endif()
# its headers as the system's, so our warnings-as-errors judge only our own code
set(GODOTCPP_SYSTEM_HEADERS ON CACHE BOOL "" FORCE)
add_subdirectory("${GODOT_CPP}" godot-cpp EXCLUDE_FROM_ALL)

add_library(samebits_gd SHARED samebits_gd.cpp ../src/maths.cpp ../src/world.cpp)
target_include_directories(samebits_gd PRIVATE ../src)
target_compile_options(samebits_gd PRIVATE -Wall -Wextra -Werror -ffp-contract=off -fno-fast-math)
target_link_libraries(samebits_gd PRIVATE godot-cpp)

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(PLATFORM android.arm64)
else()
    set(PLATFORM linux.x86_64)
endif()
set(APP_BIN "${CMAKE_CURRENT_SOURCE_DIR}/../../app/samebits/bin")
add_custom_command(TARGET samebits_gd POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APP_BIN}"
    COMMAND ${CMAKE_STRIP} -o "${APP_BIN}/libsamebits.${PLATFORM}.so" "$<TARGET_FILE:samebits_gd>"
    VERBATIM)
