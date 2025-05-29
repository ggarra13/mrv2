include(ExternalProject)

set(OpenColorIO_GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/OpenColorIO.git")
set(OpenColorIO_GIT_TAG "v2.4.2")

set(OpenColorIO_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DOCIO_BUILD_APPS=OFF
    -DOCIO_BUILD_TESTS=OFF
    -DOCIO_BUILD_GPU_TESTS=OFF
    -DOCIO_BUILD_PYTHON=OFF
    -DOCIO_INSTALL_EXT_PACKAGES=NONE)

ExternalProject_Add(
    OpenColorIO
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenColorIO
    DEPENDS Imath yaml-cpp expat pystring minizip-ng ZLIB
    GIT_REPOSITORY ${OpenColorIO_GIT_REPOSITORY}
    GIT_TAG ${OpenColorIO_GIT_TAG}
    
    LIST_SEPARATOR |
    CMAKE_ARGS ${OpenColorIO_ARGS})
