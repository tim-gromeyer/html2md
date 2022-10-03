cmake_minimum_required(VERSION 3.12)
project("html2md" VERSION 0.0.1
             DESCRIPTION "A header-only c++ library that converts HTML to Markdown."
             HOMEPAGE_URL "https://github.com/software-made-easy/html2md")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(html2md cli/main.cpp html2md.hpp)
