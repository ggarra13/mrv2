include(ExternalProject)

set(yaml-cpp_GIT_REPOSITORY "https://github.com/jbeder/yaml-cpp.git")
set(yaml-cpp_GIT_TAG "yaml-cpp-0.7.0")

# \@todo: Patch for cmake 4.0 (remove later)
set(yaml-cpp_PATCH ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/yaml-cpp-patch/CMakeLists.txt
    ${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp/src/yaml-cpp/CMakeLists.txt )
    
set(yaml-cpp_ARGS
    -DYAML_CPP_BUILD_CONTRIB=OFF
    -DYAML_CPP_BUILD_TOOLS=OFF
    -DYAML_CPP_BUILD_TESTS=OFF
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    yaml-cpp
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp
    GIT_REPOSITORY ${yaml-cpp_GIT_REPOSITORY}
    GIT_TAG ${yaml-cpp_GIT_TAG}
    PATCH_COMMAND ${yaml-cpp_PATCH}
    
    LIST_SEPARATOR |
    CMAKE_ARGS ${yaml-cpp_ARGS})
