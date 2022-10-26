project("tests" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MD4C_FILES
    ${CMAKE_CURRENT_LIST_DIR}/md4c/src/entity.c
    ${CMAKE_CURRENT_LIST_DIR}/md4c/src/entity.h
    ${CMAKE_CURRENT_LIST_DIR}/md4c/src/md4c-html.c
    ${CMAKE_CURRENT_LIST_DIR}/md4c/src/md4c-html.h
    ${CMAKE_CURRENT_LIST_DIR}/md4c/src/md4c.c
    ${CMAKE_CURRENT_LIST_DIR}/md4c/src/md4c.h
)

add_library(md4c-html STATIC ${MD4C_FILES})
target_include_directories(md4c-html PUBLIC ${CMAKE_CURRENT_LIST_DIR}/md4c/src)

set(TEST_SOURCES
    main.cpp
)

add_executable(test-exe ${TEST_SOURCES})
target_link_libraries(test-exe md4c-html html2md-static)
target_compile_definitions(test-exe PUBLIC DIR="${CMAKE_CURRENT_LIST_DIR}")

add_custom_target(tests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test-exe
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running tests"
)
add_dependencies(tests test-exe)
