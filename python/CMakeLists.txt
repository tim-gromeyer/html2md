add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/pybind11)

pybind11_add_module(pyhtml2md python/bindings.cpp ${SOURCES} ${HEADER})
target_include_directories(pyhtml2md PUBLIC include)

set_target_properties(pyhtml2md PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_compile_features(pyhtml2md PUBLIC cxx_std_11) # Require at least c++11
target_compile_definitions(pyhtml2md PRIVATE PYTHON_BINDINGS)
