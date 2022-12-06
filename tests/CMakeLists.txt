project("tests" LANGUAGES C CXX)

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

add_executable(test-exe main.cpp)
target_link_libraries(test-exe md4c-html html2md-static)
target_compile_definitions(test-exe PUBLIC DIR="${CMAKE_CURRENT_LIST_DIR}")
set_target_properties(test-exe PROPERTIES OUTPUT_NAME "tests")
target_compile_features(test-exe PUBLIC cxx_std_17) # Require at least c++17

add_custom_target(test
    COMMAND $<TARGET_FILE:test-exe>
    COMMENT Runing tests..
    DEPENDS test-exe
)
