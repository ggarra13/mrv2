# Find the lcms2 library.
#
# This module defines the following variables:
#
# * lcms2_FOUND
# * lcms2_INCLUDE_DIRS
# * lcms2_LIBRARIES
#
# This module defines the following imported targets:
#
# * lcms2::lcms2
#
# This module defines the following interfaces:
#
# * lcms2

find_path(lcms2_INCLUDE_DIR NAMES lcms2.h)
set(lcms2_INCLUDE_DIRS ${lcms2_INCLUDE_DIR})

find_library(lcms2_LIBRARY
    NAMES lcms2 liblcms2
    HINTS ${CMAKE_INSTALL_PREFIX})
set(lcms2_LIBRARIES ${lcms2_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    lcms2
    REQUIRED_VARS lcms2_INCLUDE_DIR lcms2_LIBRARY)
mark_as_advanced(lcms2_INCLUDE_DIR lcms2_LIBRARY)

if(lcms2_FOUND AND NOT TARGET lcms2::lcms2)
    add_library(lcms2::lcms2 UNKNOWN IMPORTED)
    set_target_properties(lcms2::lcms2 PROPERTIES
        IMPORTED_LOCATION "${lcms2_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS lcms2_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${lcms2_INCLUDE_DIR}")
endif()
if(lcms2_FOUND AND NOT TARGET lcms2)
    add_library(lcms2 INTERFACE)
    target_link_libraries(lcms2 INTERFACE lcms2::lcms2)
endif()
