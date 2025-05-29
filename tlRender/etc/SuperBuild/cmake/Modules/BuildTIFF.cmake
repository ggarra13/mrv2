include(ExternalProject)

set(TIFF_GIT_REPOSITORY "https://gitlab.com/libtiff/libtiff.git")
set(TIFF_GIT_TAG "v4.5.0")

set(TIFF_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -Dtiff-tools=OFF
    -Dtiff-tests=OFF
    -Dtiff-contrib=OFF
    -Dtiff-docs=OFF
    -Dzstd=OFF
    -Dlibdeflate=OFF
    -Djbig=OFF
    -Djpeg=OFF
    -Dold-jpeg=OFF
    -Djpeg12=OFF
    -Dlerc=OFF
    -Dlzma=OFF
    -Dwebp=OFF)

ExternalProject_Add(
    TIFF
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/TIFF
    DEPENDS ZLIB
    GIT_REPOSITORY ${TIFF_GIT_REPOSITORY}
    GIT_TAG ${TIFF_GIT_TAG}
    
    LIST_SEPARATOR |
    CMAKE_ARGS ${TIFF_ARGS})
