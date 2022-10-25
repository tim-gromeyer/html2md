cmake_minimum_required(VERSION 3.12)
project("html2md" VERSION 1.0.0
             DESCRIPTION "A c++ library that converts HTML to Markdown."
             HOMEPAGE_URL "https://github.com/software-made-easy/html2md"
             LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Build type
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Build type not specified. Release is used.")
    set(CMAKE_BUILD_TYPE "Release")
else()
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

# Check if it was included
get_directory_property(subproject PARENT_DIRECTORY)

# Some options
if (subproject)
    option(BUILD_EXE "Build a executable to convert html to markdown." OFF)
    option(BUILD_TEST "Build tests" OFF)
else()
    option(BUILD_EXE "Build a executable to convert html to markdown." ON)
    option(BUILD_TEST "Build tests" ON)
endif()

set(SOURCES
    src/html2md.cpp
)
set(HEADER
    include/html2md.h
)

add_library(html2md SHARED ${SOURCES})
set_target_properties(html2md PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER ${HEADER}
)
target_include_directories(html2md PRIVATE include)

if (subproject OR BUILD_EXE)
    add_library(html2md-static STATIC ${HEADER} ${SOURCES})
    target_include_directories(html2md-static PUBLIC include)
endif()

if(BUILD_EXE)
    add_executable(html2md-exe cli/main.cpp)
    target_link_libraries(html2md-exe html2md-static)
    set_target_properties(html2md-exe PROPERTIES OUTPUT_NAME "html2md")
    target_compile_definitions(html2md-exe PUBLIC VERSION="${PROJECT_VERSION}")
endif()

if(BUILD_TEST)
    add_subdirectory(tests)
endif()

if(UNIX)
    include(GNUInstallDirs)

    install(TARGETS html2md
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/html2md
    )

    configure_file(html2md.pc.in html2md.pc @ONLY)
    install(FILES ${CMAKE_BINARY_DIR}/html2md.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

    if (BUILD_EXE)
        install(TARGETS html2md-exe)
    endif()
endif()
