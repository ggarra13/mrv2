include(ExternalProject)

set(OTIO_GIT_REPOSITORY "https://github.com/PixarAnimationStudios/OpenTimelineIO.git")
set(OTIO_GIT_TAG "v0.17.0")

set(OTIO_SHARED_LIBS ON)
if(NOT BUILD_SHARED_LIBS)
    set(OTIO_SHARED_LIBS OFF)
endif()

set(OTIO_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DOTIO_FIND_IMATH=ON
    -DOTIO_SHARED_LIBS=${OTIO_SHARED_LIBS}
    -DOTIO_PYTHON_INSTALL=${TLRENDER_ENABLE_PYTHON})

if(WIN32)
    set(OTIO_C_COMPILER $ENV{NATIVE_C_COMPILER})
    set(OTIO_CXX_COMPILER $ENV{NATIVE_CXX_COMPILER})
    list(APPEND OTIO_ARGS
	-DCMAKE_C_COMPILER=${OTIO_C_COMPILER}
	-DCMAKE_CXX_COMPILER=${OTIO_CXX_COMPILER}
    )
endif()

ExternalProject_Add(
    OTIO
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OTIO
    DEPENDS Imath
    GIT_REPOSITORY ${OTIO_GIT_REPOSITORY}
    GIT_TAG ${OTIO_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${OTIO_ARGS})
