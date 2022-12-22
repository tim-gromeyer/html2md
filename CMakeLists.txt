cmake_minimum_required(VERSION 3.12)
project("html2md" VERSION 1.2.0
             DESCRIPTION "Transform your HTML into clean, easy-to-read markdown with html2md"
             HOMEPAGE_URL "https://tim-gromeyer.github.io/html2md/"
             LANGUAGES CXX)

# Build type
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Build type not specified. Release is used.")
    set(CMAKE_BUILD_TYPE "Release")
else()
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

# Improve performance
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    string(REPLACE "-O2" "-O3" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REPLACE "-O2" "-O3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

# Check if it was included
get_directory_property(subproject PARENT_DIRECTORY)

# Create HTML for webassembly
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# Some options
if (subproject)
    option(BUILD_EXE "Build a executable to convert html to markdown." OFF)
    option(BUILD_DOC "Build documentation" OFF)
else()
    option(BUILD_EXE "Build a executable to convert html to markdown." ON)
    option(BUILD_DOC "Build documentation" ON)
endif()
option(BUILD_TEST "Build tests" OFF)
option(PYTHON_BINDINGS "Build python bindings" OFF)

set(SOURCES
    src/html2md.cpp
)
set(HEADERS
    include/html2md.h
)

if(PYTHON_BINDINGS)
    include(python/CMakeLists.txt)
    return()
endif()

add_library(html2md SHARED ${SOURCES})
set_target_properties(html2md PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${HEADERS}"
)
target_include_directories(html2md PRIVATE include)
target_compile_features(html2md PUBLIC cxx_std_11) # Require at least c++11

if (subproject OR BUILD_EXE)
    add_library(html2md-static STATIC ${HEADERS} ${SOURCES})
    target_include_directories(html2md-static PUBLIC include)
    target_compile_features(html2md-static PUBLIC cxx_std_11) # Require at least c++11
endif()

if(BUILD_EXE)
    add_executable(html2md-exe cli/main.cpp)
    target_link_libraries(html2md-exe html2md-static)
    set_target_properties(html2md-exe PROPERTIES OUTPUT_NAME "html2md")
    target_compile_definitions(html2md-exe PUBLIC VERSION="${PROJECT_VERSION}")
    target_compile_features(html2md-exe PUBLIC cxx_std_11) # Require at least c++11
endif()

if(BUILD_TEST)
    add_subdirectory(tests)
endif()

if(BUILD_DOC)
    include(cmake/Doc.cmake)
endif()

# Don't install as a subproject
if(subproject)
    return()
endif()

include(GNUInstallDirs)

install(TARGETS html2md
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/html2md
)

configure_file(html2md.pc.in html2md.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/html2md.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

if (BUILD_EXE)
    install(TARGETS html2md-exe DESTINATION bin)
endif()

include(cmake/Packaging.cmake)
