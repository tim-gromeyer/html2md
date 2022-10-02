cmake_minimum_required(VERSION 3.9)
project(html2md)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)

add_executable(html2md example.cpp html2md.hpp)
