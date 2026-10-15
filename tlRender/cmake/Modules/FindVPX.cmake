# Find the libpng library.
#
# This module defines the following variables:
#
# * VPX_INCLUDE_DIRS
# * VPX_LIBRARIES
#
# This module defines the following imported targets:
#
# * VPX::VPX
#
# This module defines the following interfaces:
#
# * VPX


find_path(VPX_INCLUDE_DIR NAMES vp8.h)
set(VPX_INCLUDE_DIRS ${VPX_INCLUDE_DIR})

if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    find_library(VPX_LIBRARY
        NAMES vpxd libvpxd vpxd
	HINTS ${CMAKE_INSTALL_PREFIX})
else()
    find_library(VPX_LIBRARY
        NAMES vpx libvpx vpx
	HINTS ${CMAKE_INSTALL_PREFIX})
endif()
set(VPX_LIBRARIES ${VPX_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    VPX
    REQUIRED_VARS VPX_INCLUDE_DIR VPX_LIBRARY)
mark_as_advanced(VPX_INCLUDE_DIR VPX_LIBRARY)

if(VPX_FOUND AND NOT TARGET VPX::VPX)
    add_library(VPX::VPX UNKNOWN IMPORTED)
    set_target_properties(VPX::VPX PROPERTIES
        IMPORTED_LOCATION "${VPX_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS VPX_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${VPX_INCLUDE_DIR}")
endif()
if(VPX_FOUND AND NOT TARGET VPX)
    add_library(VPX INTERFACE)
    target_link_libraries(VPX INTERFACE VPX::VPX)
endif()
