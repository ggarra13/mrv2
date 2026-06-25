# Find the libpng library.
#
# This module defines the following variables:
#
# * libssh2_INCLUDE_DIRS
# * libssh2_LIBRARIES
#
# This module defines the following imported targets:
#
# * libssh2::libssh2
#
# This module defines the following interfaces:
#
# * libssh2

find_path(libssh2_INCLUDE_DIR NAMES libssh2.h)
set(libssh2_INCLUDE_DIRS
    ${libssh2_INCLUDE_DIR})

find_library(libssh2_LIBRARY
    NAMES ssh2 libssh2
    HINTS ${CMAKE_INSTALL_PREFIX})
set(libssh2_LIBRARIES
    ${libssh2_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    libssh2
    REQUIRED_VARS libssh2_INCLUDE_DIR libssh2_LIBRARY)
mark_as_advanced(libssh2_INCLUDE_DIR libssh2_LIBRARY)

if(libssh2_FOUND AND NOT TARGET libssh2::libssh2)
    add_library(libssh2::libssh2 UNKNOWN IMPORTED)
    set_target_properties(libssh2::libssh2 PROPERTIES
        IMPORTED_LOCATION "${libssh2_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS libssh2_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${libssh2_INCLUDE_DIR}")
endif()
if(libssh2_FOUND AND NOT TARGET libssh2)
    add_library(libssh2 INTERFACE)
    target_link_libraries(libssh2 INTERFACE libssh2::libssh2)
endif()
