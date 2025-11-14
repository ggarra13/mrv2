include(ExternalProject)

set(OTIO_GIT_REPOSITORY "https://github.com/PixarAnimationStudios/OpenTimelineIO.git")
if(UNIX)
    set(OTIO_GIT_TAG "v0.18.1") # currently, not Windows friendly.
else()
    set(OTIO_GIT_TAG "v0.17.0") # last Windows friendly version.
endif()

set(OTIO_DEPENDENCIES Imath)
message(STATUS "OTIO DEPENDENCIES=${OTIO_DEPENDENCIES}")

set(OTIO_SHARED_LIBS ON)
if(NOT BUILD_SHARED_LIBS)
    set(OTIO_SHARED_LIBS OFF)
endif()

set(OTIO_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DOTIO_FIND_IMATH=ON
    -DOTIO_SHARED_LIBS=${OTIO_SHARED_LIBS}
    -DOTIO_PYTHON_INSTALL=${TLRENDER_ENABLE_PYTHON})

set(OTIO_PATCH )

if(WIN32 AND OTIO_GIT_TAG STREQUAL "v0.18.1")
    list(APPEND OTIO_PATCH
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_CURRENT_SOURCE_DIR}/patches/OTIO-patch/src/opentime/export.h
	${CMAKE_CURRENT_BINARY_DIR}/OTIO/src/OTIO/src/opentime/export.h)
    list(APPEND OTIO_PATCH
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_CURRENT_SOURCE_DIR}/patches/OTIO-patch/src/opentimelineio/export.h
	${CMAKE_CURRENT_BINARY_DIR}/OTIO/src/OTIO/src/opentimelineio/export.h)
endif()

ExternalProject_Add(
    OTIO
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OTIO
    DEPENDS ${OTIO_DEPENDENCIES}
    PATCH_COMMAND  ${OTIO_PATCH}
    GIT_REPOSITORY ${OTIO_GIT_REPOSITORY}
    GIT_TAG ${OTIO_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${OTIO_ARGS})
