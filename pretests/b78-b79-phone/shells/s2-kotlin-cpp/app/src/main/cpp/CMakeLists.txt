# B78 shell 2: the phone library, built from the shared C++ core.
cmake_minimum_required(VERSION 3.22)
project(shell2 CXX)
add_library(shell2 SHARED jni.cpp)
target_include_directories(shell2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../core)
target_link_options(shell2 PRIVATE "-Wl,-z,max-page-size=16384")
