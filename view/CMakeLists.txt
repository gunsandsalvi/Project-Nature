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

# What needs no Godot, so its tests run alone: the speed loop, the heat governor, the crowd's stepper and buffers, the
# frame meter, the look's camera rig, gesture reader, scripted paths, stage and texture files, the poser, and the model
# kit's parts, checks and assembler, the stand-in area's land, and the view's maps round things.
add_library(kd_view_core STATIC src/pace.cpp src/heat.cpp src/crowd_core.cpp src/frames.cpp src/gestures.cpp src/rig.cpp
    src/projection.cpp src/terrain.cpp src/path.cpp src/stage.cpp src/textures.cpp src/poser.cpp src/kit.cpp src/kit_assemble.cpp src/area.cpp src/maps.cpp src/patches.cpp)
target_include_directories(kd_view_core PUBLIC src)
target_link_libraries(kd_view_core PUBLIC kd_sim)
kindling_rules(kd_view_core)

add_library(kindling SHARED
    src/register.cpp
    src/canvas.cpp
    src/terrain_draw.cpp
    src/area_draw.cpp
    src/calibration.cpp
    src/crowd.cpp
    src/device.cpp
    src/figures.cpp
    src/kit_draw.cpp
    src/look.cpp
    src/maps_draw.cpp
    src/texture_images.cpp
    src/world.cpp
    src/worlds.cpp)
kindling_rules(kindling)
target_link_libraries(kindling PRIVATE kd_view_core kd_sim godot-cpp)
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

# The kit's own tool, which the build and the cloud's checks run over the parts and their recipes (A6.1, A6.4), and
# which needs no Godot: the cloud's build only.
if(NOT ANDROID)
    add_executable(kd_kit tools/kit.cpp)
    target_link_libraries(kd_kit PRIVATE kd_view_core)
    kindling_rules(kd_kit)
endif()

# The extension's own tests, of what needs no Godot, on the cloud's build only.
option(KD_VIEW_TESTS "Build the extension's own tests" ON)
if(KD_VIEW_TESTS AND NOT ANDROID)
    set(KD_DOCTEST "$ENV{KD_DOCTEST}" CACHE PATH "doctest's folder (tools/setup.sh)")
    add_executable(kd_view_tests tests/main.cpp tests/pace_test.cpp tests/area_test.cpp tests/crowd_core_test.cpp
        tests/projection_test.cpp tests/terrain_test.cpp tests/frames_test.cpp tests/heat_test.cpp tests/rig_test.cpp tests/stage_test.cpp tests/poser_test.cpp tests/kit_test.cpp tests/maps_test.cpp tests/patches_test.cpp)
    target_include_directories(kd_view_tests SYSTEM PRIVATE "${KD_DOCTEST}")
    target_compile_definitions(kd_view_tests PRIVATE DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS)
    target_link_libraries(kd_view_tests PRIVATE kd_view_core)
    kindling_rules(kd_view_tests)
    enable_testing()
    add_test(NAME view COMMAND kd_view_tests)
endif()
