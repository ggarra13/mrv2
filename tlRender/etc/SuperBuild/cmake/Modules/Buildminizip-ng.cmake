include(ExternalProject)

set(minizip-ng_GIT_REPOSITORY "https://github.com/zlib-ng/minizip-ng.git")
set(minizip-ng_GIT_TAG "3.0.7")

set(minizip-ng_ARGS
    -DMZ_BZIP2=OFF
    -DMZ_LZMA=OFF
    -DMZ_ZSTD=OFF
    -DMZ_LIBCOMP=OFF
    -DMZ_FETCH_LIBS=OFF
    -DMZ_PKCRYPT=OFF
    -DMZ_WZAES=OFF
    -DMZ_OPENSSL=OFF
    -DMZ_BCRYPT=OFF
    -DMZ_LIBBSD=OFF
    -DMZ_ICONV=OFF
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    minizip-ng
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/minizip-ng
    DEPENDS ZLIB
    GIT_REPOSITORY ${minizip-ng_GIT_REPOSITORY}
    GIT_TAG ${minizip-ng_GIT_TAG}
    
    LIST_SEPARATOR |
    CMAKE_ARGS ${minizip-ng_ARGS})
