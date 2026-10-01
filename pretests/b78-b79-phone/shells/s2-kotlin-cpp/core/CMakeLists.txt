# Headless Linux build of the shared C++ core.
cmake_minimum_required(VERSION 3.22)
project(kcore_headless CXX)
add_executable(headless headless.cpp)
