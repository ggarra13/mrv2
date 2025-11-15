include(ExternalProject)

set(ZLIB_GIT_REPOSITORY "https://github.com/madler/zlib.git")
set(ZLIB_GIT_TAG "v1.3.1")

set(ZLIB_PATCH ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/patches/ZLIB-patch/CMakeLists.txt
    ${CMAKE_CURRENT_BINARY_DIR}/ZLIB/src/ZLIB/CMakeLists.txt)
if(WIN32)
    list(APPEND ZLIB_PATCH COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/patches/ZLIB-patch/zconf.h.cmakein
       ${CMAKE_CURRENT_BINARY_DIR}/ZLIB/src/ZLIB/zconf.h.cmakein)
endif()

set(ZLIB_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DSKIP_INSTALL_FILES=ON
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON)

ExternalProject_Add(
    ZLIB
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ZLIB
    GIT_REPOSITORY ${ZLIB_GIT_REPOSITORY}
    GIT_TAG ${ZLIB_GIT_TAG}
    PATCH_COMMAND  ${ZLIB_PATCH}
    LIST_SEPARATOR |
    CMAKE_ARGS ${ZLIB_ARGS})
