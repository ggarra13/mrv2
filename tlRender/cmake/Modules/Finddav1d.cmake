# Find the dav1d library.
#
# This module defines the following variables:
#
# * dav1d_FOUND
# * dav1d_INCLUDE_DIRS
# * dav1d_LIBRARIES
#
# This module defines the following imported targets:
#
# * dav1d::dav1d
#
# This module defines the following interfaces:
#
# * dav1d

find_path(dav1d_INCLUDE_DIR NAMES dav1d/dav1d.h)
set(dav1d_INCLUDE_DIRS ${dav1d_INCLUDE_DIR})

find_library(dav1d_LIBRARY
    NAMES dav1d libdav1d
    HINTS ${CMAKE_INSTALL_PREFIX})
set(dav1d_LIBRARIES ${dav1d_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    dav1d
    REQUIRED_VARS dav1d_INCLUDE_DIR dav1d_LIBRARY)
mark_as_advanced(dav1d_INCLUDE_DIR dav1d_LIBRARY)

if(dav1d_FOUND AND NOT TARGET dav1d::dav1d)
    add_library(dav1d::dav1d UNKNOWN IMPORTED)
    set_target_properties(dav1d::dav1d PROPERTIES
        IMPORTED_LOCATION "${dav1d_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS dav1d_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${dav1d_INCLUDE_DIR}")
endif()
if(dav1d_FOUND AND NOT TARGET dav1d)
    add_library(dav1d INTERFACE)
    target_link_libraries(dav1d INTERFACE dav1d::dav1d)
endif()
