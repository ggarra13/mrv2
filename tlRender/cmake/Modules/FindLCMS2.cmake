# Find the zlib library.
#
# This module defines the following variables:
#
# * LCMS2_VERSION
# * LCMS2_INCLUDE_DIRS
# * LCMS2_LIBRARIES
#
# This module defines the following imported targets:
#
# * LCMS2::lcms2
# * LCMS2::LCMS2
#
# This module defines the following interfaces:
#
# * LCMS2


find_path(LCMS2_INCLUDE_DIR NAMES lcms2.h)
set(LCMS2_INCLUDE_DIRS ${LCMS2_INCLUDE_DIR})

if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    find_library(LCMS2_LIBRARY NAMES lcms2d lcms2 liblcms2d liblcms2)
else()
    find_library(LCMS2_LIBRARY NAMES lcms2 liblcms2)
endif()
set(LCMS2_LIBRARIES ${LCMS2_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    LCMS2
    REQUIRED_VARS LCMS2_INCLUDE_DIR LCMS2_LIBRARY
    VERSION_VAR LCMS2_VERSION)
mark_as_advanced(LCMS2_INCLUDE_DIR LCMS2_LIBRARY)

if(LCMS2_FOUND AND NOT TARGET LCMS2::lcms2)
    add_library(LCMS2::lcms2 UNKNOWN IMPORTED)
    set_target_properties(LCMS2::lcms2 PROPERTIES
        IMPORTED_LOCATION "${LCMS2_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS LCMS2_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${LCMS2_INCLUDE_DIR}")
endif()
if(LCMS2_FOUND AND NOT TARGET LCMS2::LCMS2)
    add_library(LCMS2::LCMS2 UNKNOWN IMPORTED)
    set_target_properties(LCMS2::LCMS2 PROPERTIES
        IMPORTED_LOCATION "${LCMS2_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS LCMS2_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${LCMS2_INCLUDE_DIR}")
endif()
if(LCMS2_FOUND AND NOT TARGET LCMS2)
    add_library(LCMS2 INTERFACE)
    target_link_libraries(LCMS2 INTERFACE LCMS2::lcms2)
endif()
