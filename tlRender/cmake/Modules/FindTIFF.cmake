# Find the libTIFF library.
#
# This module defines the following variables:
#
# * TIFF_VERSION
# * TIFF_INCLUDE_DIRS
# * TIFF_LIBRARIES
#
# This module defines the following imported targets:
#
# * TIFF::tiff
#
# This module defines the following interfaces:
#
# * TIFF

find_package(ZLIB REQUIRED)
find_package(libjpeg-turbo)

set(TIFF_VERSION 4.1.0)

find_path(TIFF_INCLUDE_DIR NAMES tiff.h)
set(TIFF_INCLUDE_DIRS ${TIFF_INCLUDE_DIR})
if(libjpeg-turbo_FOUND)
    list(APPEND TIFF_INCLUDE_DIRS ${libjpeg-turbo_INCLUDE_DIRS})
endif()
list(APPEND TIFF_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})

if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    find_library(TIFF_LIBRARY NAMES tiffd tiff)
else()
    find_library(TIFF_LIBRARY NAMES tiff)
endif()
set(TIFF_LIBRARIES ${TIFF_LIBRARY})
if(libjpeg-turbo_FOUND)
    list(APPEND TIFF_LIBRARIES ${libjpeg-turbo_LIBRARIES})
endif()
list(APPEND TIFF_LIBRARIES ${ZLIB_LIBRARIES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    TIFF
    REQUIRED_VARS TIFF_INCLUDE_DIR TIFF_LIBRARY)
mark_as_advanced(TIFF_INCLUDE_DIR TIFF_LIBRARY)

if(TIFF_FOUND AND NOT TARGET TIFF::tiff)
    add_library(TIFF::tiff UNKNOWN IMPORTED)
    set(TIFF_INTERFACE_LINK_LIBRARIES)
    if(libjpeg-turbo_FOUND)
        list(APPEND TIFF_INTERFACE_LINK_LIBRARIES libjpeg-turbo::turbojpeg-static)
    endif()
    list(APPEND TIFF_INTERFACE_LINK_LIBRARIES ZLIB)
    set_target_properties(TIFF::tiff PROPERTIES
        IMPORTED_LOCATION "${TIFF_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS TIFF_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${TIFF_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${TIFF_INTERFACE_LINK_LIBRARIES}")
endif()
if(TIFF_FOUND AND NOT TARGET TIFF)
    add_library(TIFF INTERFACE)
    target_link_libraries(TIFF INTERFACE TIFF::tiff)
endif()
