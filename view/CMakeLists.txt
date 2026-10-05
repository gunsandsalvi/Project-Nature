# The app's one Godot extension, libkindling (A2.1, A2.2): the bridge to the simulation, built with godot-cpp 4.5
# from a trimmed profile, for Linux x86-64 (the cloud's Godot tests) and Android arm64 (the phone).
# The built library goes to game/bin/, stripped for the phone, and is never committed.
cmake_minimum_required(VERSION 3.22)
project(kindling_view LANGUAGES C CXX)

set(KD_GODOT_CPP "$ENV{KD_GODOT_CPP}" CACHE PATH "godot-cpp's folder (tools/setup.sh)")

# The simulation's library alone, without its tests and tool.
set(KD_SIM_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(../sim sim)

set(GODOTCPP_BUILD_PROFILE "${CMAKE_CURRENT_SOURCE_DIR}/build_profile.json" CACHE PATH "" FORCE)
set(GODOTCPP_TARGET template_release CACHE STRING "" FORCE)
set(GODOTCPP_SYSTEM_HEADERS ON CACHE BOOL "" FORCE)
add_subdirectory("${KD_GODOT_CPP}" godot-cpp EXCLUDE_FROM_ALL)

add_library(kindling SHARED
    src/register.cpp
    src/device.cpp)
kindling_rules(kindling)
target_link_libraries(kindling PRIVATE kd_sim godot-cpp)
target_link_options(kindling PRIVATE -Wl,--gc-sections -Wl,--exclude-libs,ALL)

if(ANDROID)
    set(KD_PLATFORM android.arm64)
    target_link_libraries(kindling PRIVATE android log)
    set(KD_STRIP "${ANDROID_TOOLCHAIN_ROOT}/bin/llvm-strip")
else()
    set(KD_PLATFORM linux.x86_64)
    set(KD_STRIP "${CMAKE_STRIP}")
endif()
set_target_properties(kindling PROPERTIES OUTPUT_NAME "kindling.${KD_PLATFORM}")

# The game loads the library from game/bin/; the copy there is stripped, the one in the build folder keeps its
# symbols for reading crash reports.
set(KD_GAME_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../game/bin/libkindling.${KD_PLATFORM}.so")
add_custom_command(TARGET kindling POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/../game/bin"
    COMMAND "${KD_STRIP}" --strip-unneeded -o "${KD_GAME_LIB}" "$<TARGET_FILE:kindling>"
    VERBATIM)
