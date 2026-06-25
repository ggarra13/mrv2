# Find the svt-av1 library.
#
# This module defines the following variables:
#
# * svt-av1_FOUND
# * svt-av1_INCLUDE_DIRS
# * svt-av1_LIBRARIES
#
# This module defines the following imported targets:
#
# * svt-av1::svt-av1
#
# This module defines the following interfaces:
#
# * svt-av1

find_path(svt-av1_INCLUDE_DIR NAMES svt-av1/EbSvtAv1.h)
set(svt-av1_INCLUDE_DIRS ${svt-av1_INCLUDE_DIR})

find_library(svt-av1_LIBRARY
    NAMES SvtAv1Enc libSvtAv1Enc
    HINTS ${CMAKE_INSTALL_PREFIX})
set(svt-av1_LIBRARIES ${svt-av1_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    svt-av1
    REQUIRED_VARS svt-av1_INCLUDE_DIR svt-av1_LIBRARY)
mark_as_advanced(svt-av1_INCLUDE_DIR svt-av1_LIBRARY)

if(svt-av1_FOUND AND NOT TARGET svt-av1::svt-av1)
    add_library(svt-av1::svt-av1 UNKNOWN IMPORTED)
    set_target_properties(svt-av1::svt-av1 PROPERTIES
        IMPORTED_LOCATION "${svt-av1_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS svt-av1_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${svt-av1_INCLUDE_DIR}")
endif()
if(svt-av1_FOUND AND NOT TARGET svt-av1)
    add_library(svt-av1 INTERFACE)
    target_link_libraries(svt-av1 INTERFACE svt-av1::svt-av1)
endif()
