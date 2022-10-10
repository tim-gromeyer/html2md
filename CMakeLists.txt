cmake_minimum_required(VERSION 3.12)
project("html2md" VERSION 0.0.1
             DESCRIPTION "A c++ library that converts HTML to Markdown."
             HOMEPAGE_URL "https://github.com/software-made-easy/html2md"
             LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Some options
option(BUILD_EXE "Build a executable to convert html to markdown." ON)

add_library(html2md src/html2md.cpp include/html2md.h)
target_include_directories(html2md PUBLIC include src)

if(BUILD_EXE)
    add_executable(html2md-exe cli/main.cpp)
    target_link_libraries(html2md-exe html2md)
endif()

if(UNIX AND BUID_EXE)
    include(GNUInstallDirs)

    install(TARGETS html2md-exe)
endif()
