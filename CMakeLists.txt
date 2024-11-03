# Minimum CMake version requirement
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(DominatingSetProblem VERSION 1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable target
add_executable(main main.cpp graph.cpp)
