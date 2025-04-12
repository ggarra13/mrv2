include(ExternalProject)

set(pystring_GIT_REPOSITORY "https://github.com/imageworks/pystring.git")
set(pystring_GIT_TAG "v1.1.4")

set(pystring_ARGS
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    pystring
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/pystring
    GIT_REPOSITORY ${pystring_GIT_REPOSITORY}
    GIT_TAG ${pystring_GIT_TAG}
    
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/pystring-patch/CMakeLists.txt
        ${CMAKE_CURRENT_BINARY_DIR}/pystring/src/pystring/CMakeLists.txt
    LIST_SEPARATOR |
    CMAKE_ARGS ${pystring_ARGS})
