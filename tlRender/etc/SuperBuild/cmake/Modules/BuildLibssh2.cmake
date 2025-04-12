include(ExternalProject)

set(Libssh2_GIT_REPOSITORY "https://github.com/libssh2/libssh2.git")
set(Libssh2_GIT_TAG "libssh2-1.11.0")

set(Libssh2_ARGS
    -DCRYPTO_BACKEND=OpenSSL
    -DBUILD_EXAMPLES=OFF
    -DBUILD_TESTING=OFF
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    Libssh2
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/Libssh2
    DEPENDS ${OpenSSL} ZLIB
    GIT_REPOSITORY ${Libssh2_GIT_REPOSITORY}
    GIT_TAG ${Libssh2_GIT_TAG}
    
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/Libssh2-patch/CMakeLists.txt
        ${CMAKE_CURRENT_BINARY_DIR}/Libssh2/src/Libssh2/CMakeLists.txt
    LIST_SEPARATOR |
    CMAKE_ARGS ${Libssh2_ARGS})
