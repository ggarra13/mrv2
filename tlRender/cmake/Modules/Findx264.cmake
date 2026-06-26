# Find the libpng library.
#
# This module defines the following variables:
#
# * x264_INCLUDE_DIRS
# * x264_LIBRARIES
#
# This module defines the following imported targets:
#
# * x264::x264
#
# This module defines the following interfaces:
#
# * x264


find_path(x264_INCLUDE_DIR NAMES x264.h)
set(x264_INCLUDE_DIRS ${x264_INCLUDE_DIR})

find_library(x264_LIBRARY
    NAMES x264 libx264
    HINTS ${CMAKE_INSTALL_PREFIX})
set(x264_LIBRARIES ${x264_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    x264
    REQUIRED_VARS x264_INCLUDE_DIR x264_LIBRARY)
mark_as_advanced(x264_INCLUDE_DIR x264_LIBRARY)

if(x264_FOUND AND NOT TARGET x264::x264)
    add_library(x264::x264 UNKNOWN IMPORTED)
    set_target_properties(x264::x264 PROPERTIES
        IMPORTED_LOCATION "${x264_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS x264_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${x264_INCLUDE_DIR}")
endif()
if(x264_FOUND AND NOT TARGET x264)
    add_library(x264 INTERFACE)
    target_link_libraries(x264 INTERFACE x264::x264)
endif()
