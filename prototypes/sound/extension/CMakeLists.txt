# P14's Godot extension (IMPLEMENTATION α0.7c): the sound library and godot-cpp, pinned at its 4.5 release, which Godot
# 4.7 loads, built into one shared library: natively for the cloud's tests, and for the phone's arm64 Android by
# tools/build.sh with the NDK. Each build puts its library where the app's sound.gdextension looks for it, so nothing
# built is committed. Pre-production code (research 00).
cmake_minimum_required(VERSION 3.22)
project(sound_gd CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GODOTCPP_BUILD_PROFILE "${CMAKE_CURRENT_SOURCE_DIR}/build_profile.json" CACHE PATH "" FORCE)
if(NOT GODOTCPP_TARGET)
    set(GODOTCPP_TARGET template_release CACHE STRING "" FORCE)
endif()
# its headers as the system's, so our warnings-as-errors judge only our own code
set(GODOTCPP_SYSTEM_HEADERS ON CACHE BOOL "" FORCE)
add_subdirectory("${GODOT_CPP}" godot-cpp EXCLUDE_FROM_ALL)

add_library(sound_gd SHARED sound_gd.cpp ../src/sound.cpp)
target_include_directories(sound_gd PRIVATE ../src)
target_compile_options(sound_gd PRIVATE -Wall -Wextra -Werror)
target_link_libraries(sound_gd PRIVATE godot-cpp)

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(PLATFORM android.arm64)
else()
    set(PLATFORM linux.x86_64)
endif()
set(APP_BIN "${CMAKE_CURRENT_SOURCE_DIR}/../../app/sound/bin")
add_custom_command(TARGET sound_gd POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APP_BIN}"
    COMMAND ${CMAKE_STRIP} -o "${APP_BIN}/libsound.${PLATFORM}.so" "$<TARGET_FILE:sound_gd>"
    VERBATIM)
