# Find the native file dialog extended library.
#
# This module defines the following variables:
#
# * NFD_INCLUDE_DIRS
# * NFD_LIBRARIES
#
# This module defines the following imported targets:
#
# * NFD::NFD
#
# This module defines the following interfaces:
#
# * NFD

if(WIN32)
elseif(APPLE)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
endif()

find_path(NFD_INCLUDE_DIR NAMES nfd.h)
set(NFD_INCLUDE_DIRS
    ${NFD_INCLUDE_DIR}
    ${GTK3_INCLUDE_DIRS})

find_library(NFD_LIBRARY NAMES nfd)
set(NFD_LIBRARIES
    ${NFD_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    NFD
    REQUIRED_VARS NFD_INCLUDE_DIR NFD_LIBRARY)
mark_as_advanced(NFD_INCLUDE_DIR NFD_LIBRARY)

set(NFD_COMPILE_DEFINITIONS NFD_FOUND)

set(NFD_LINK_LIBRARIES)
set(NFD_LINK_LIBRARIES)
if(WIN32)
    list(APPEND NFD_LINK_LIBRARIES ole32 uuid shell32)
elseif(APPLE)
    list(APPEND NFD_LINK_LIBRARIES "-framework AppKit")
else()
    list(APPEND NFD_LINK_LIBRARIES ${GTK3_LINK_LIBRARIES})
endif()

if(NFD_FOUND AND NOT TARGET NFD::NFD)
    add_library(NFD::NFD UNKNOWN IMPORTED)
    set_target_properties(NFD::NFD PROPERTIES
        IMPORTED_LOCATION "${NFD_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${NFD_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${NFD_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${NFD_LINK_LIBRARIES}")
endif()
if(NFD_FOUND AND NOT TARGET NFD)
    add_library(NFD INTERFACE)
    target_link_libraries(NFD INTERFACE NFD::NFD)
endif()
