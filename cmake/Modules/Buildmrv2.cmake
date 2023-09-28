# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include(ExternalProject)


set(mrv2_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHIECTURES}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
    -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_MESSAGE=${CMAKE_INSTALL_MESSAGE}
    -DTLRENDER_VERSION=${TLRENDER_VERSION}
    -DTLRENDER_MMAP=${TLRENDER_MMAP}
    -DTLRENDER_PYTHON=${TLRENDER_PYTHON}
    -DTLRENDER_OCIO=${TLRENDER_OCIO}
    -DTLRENDER_AUDIO=${TLRENDER_AUDIO}
    -DTLRENDER_FREETYPE=${TLRENDER_FREETYPE}
    -DTLRENDER_JPEG=${TLRENDER_JPEG}
    -DTLRENDER_TIFF=${TLRENDER_TIFF}
    -DTLRENDER_PNG=${TLRENDER_PNG}
    -DTLRENDER_EXR=${TLRENDER_EXR}
    -DTLRENDER_USD=${TLRENDER_USD}
    -DTLRENDER_API=${TLRENDER_API}
    -DTLRENDER_FFMPEG=${TLRENDER_FFMPEG}
    -DTLRENDER_GL=${TLRENDER_GL}
    -DTLRENDER_BMD=${TLRENDER_BMD}
    -DTLRENDER_BMD_SDK=${TLRENDER_BMD_SDK}
    -DBUILD_PYTHON=${BUILD_PYTHON}
    -DMRV2_NETWORK=${MRV2_NETWORK}
    -DMRV2_PYBIND11=${MRV2_PYBIND11}
    -DMRV2_PDF=${MRV2_PDF}
)

ExternalProject_Add(
    mrv2
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/mrv2
    DEPENDS tlRender FLTK ${BOOST} ${Gettext} ${ZLIB_SHARED}
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/mrv2
    LIST_SEPARATOR |
    CMAKE_ARGS ${mrv2_ARGS}
    DEPENDS ${PYBIND11} ${POCO_DEP} ${LibHaru_DEP} tlRender
)
