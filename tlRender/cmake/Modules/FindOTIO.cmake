# Find the OpenTimelineIO library.
#
# This module defines the following variables:
#
# * OTIO_INCLUDE_DIRS
# * OTIO_LIBRARIES
#
# This module defines the following imported targets:
#
# * OTIO::opentime
# * OTIO::opentimelineio
#
# This module defines the following interfaces:
#
# * OTIO

find_path(OTIO_INCLUDE_DIR NAMES opentimelineio/version.h)
if(OTIO_INCLUDE_DIR)
    set(OTIO_DEPS_INCLUDE_DIRS ${OTIO_INCLUDE_DIR}/opentimelineio/deps)
endif()
set(OTIO_INCLUDE_DIRS ${OTIO_INCLUDE_DIR} ${OTIO_DEPS_INCLUDE_DIRS})

if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    find_library(
        opentime_LIBRARY
        NAMES opentimed opentime
        PATHS ${CMAKE_INSTALL_PREFIX}/python/opentimelineio)
    find_library(
        opentimelineio_LIBRARY
        NAMES opentimelineiod opentimelineio
        PATHS ${CMAKE_INSTALL_PREFIX}/python/opentimelineio)
else()
    find_library(
        opentime_LIBRARY
        NAMES opentime
        PATHS ${CMAKE_INSTALL_PREFIX}/python/opentimelineio)
    find_library(
        opentimelineio_LIBRARY
        NAMES opentimelineio
        PATHS ${CMAKE_INSTALL_PREFIX}/python/opentimelineio)
endif()
set(OTIO_LIBRARIES ${opentimelineio_LIBRARY} ${opentime_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    OTIO
    REQUIRED_VARS OTIO_INCLUDE_DIR OTIO_DEPS_INCLUDE_DIRS opentimelineio_LIBRARY opentime_LIBRARY)
mark_as_advanced(OTIO_INCLUDE_DIR OTIO_DEPS_INCLUDE_DIRS opentimelineio_LIBRARY opentime_LIBRARY)

if(OTIO_FOUND AND NOT TARGET OTIO::opentime)
    add_library(OTIO::opentime UNKNOWN IMPORTED)
    set_target_properties(OTIO::opentime PROPERTIES
        IMPORTED_LOCATION "${opentime_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS opentime_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${opentime_INCLUDE_DIR}")
endif()
if(OTIO_FOUND AND NOT TARGET OTIO::opentimelineio)
    add_library(OTIO::opentimelineio UNKNOWN IMPORTED)
    set_target_properties(OTIO::opentimelineio PROPERTIES
        IMPORTED_LOCATION "${opentimelineio_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS opentimelineio_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${OTIO_INCLUDE_DIR};${OTIO_DEPS_INCLUDE_DIRS}")
endif()
if(OTIO_FOUND AND NOT TARGET OTIO)
    add_library(OTIO INTERFACE)
    target_link_libraries(OTIO INTERFACE OTIO::opentimelineio OTIO::opentime)
endif()
