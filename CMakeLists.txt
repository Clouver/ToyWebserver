cmake_minimum_required(VERSION 3.22)
project(ToyWebserver)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(ToyWebserver ${SOURCE_FILES})
