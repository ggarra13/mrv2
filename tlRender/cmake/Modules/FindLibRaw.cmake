# Find the libraw library.
#
# This module defines the following variables:
#
# * LibRaw_FOUND
# * LibRaw_INCLUDE_DIRS
# * LibRaw_LIBRARIES
#
# This module defines the following imported targets:
#
# * LibRaw::libraw
#
# This module defines the following interfaces:
#
# * LibRaw

find_package(ZLIB REQUIRED)
find_package(OpenMP)
find_package(LCMS2)

find_path(LibRaw_INCLUDE_DIR NAMES libraw/libraw.h)
set(LibRaw_INCLUDE_DIRS
    ${LibRaw_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS})

if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    find_library(LibRaw_LIBRARY
        NAMES rawd raw)
    find_library(LibRaw_r_LIBRARY
        NAMES raw_rd raw_r)
else()
    find_library(LibRaw_LIBRARY
        NAMES raw)
    find_library(LibRaw_r_LIBRARY
        NAMES raw_r)
endif()
set(LibRaw_LIBRARIES
    ${LibRaw_LIBRARY}
    ${ZLIB_LIBRARIES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    LibRaw
    REQUIRED_VARS LibRaw_INCLUDE_DIR LibRaw_LIBRARY)
mark_as_advanced(LibRaw_INCLUDE_DIR LibRaw_LIBRARY)

set(LibRaw_LINK_LIBRARIES ZLIB)
if(LCMS2_FOUND)
    list(APPEND LibRaw_LINK_LIBRARIES LCMS2)
endif()
if(OpenMP_FOUND)
    list(APPEND LibRaw_LINK_LIBRARIES OpenMP::OpenMP_CXX)
endif()

if(LibRaw_FOUND AND NOT TARGET LibRaw::libraw)
    add_library(LibRaw::libraw UNKNOWN IMPORTED)
    set_target_properties(LibRaw::libraw PROPERTIES
        IMPORTED_LOCATION "${LibRaw_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS LibRaw_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${LibRaw_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${LibRaw_LINK_LIBRARIES}")
endif()
if(LibRaw_FOUND AND NOT TARGET LibRaw::libraw_r)
    add_library(LibRaw::libraw_r UNKNOWN IMPORTED)
    set_target_properties(LibRaw::libraw_r PROPERTIES
        IMPORTED_LOCATION "${LibRaw_r_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS LibRaw_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${LibRaw_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${LibRaw_LINK_LIBRARIES}")
endif()
if(LibRaw_FOUND AND NOT TARGET LibRaw)
    add_library(LibRaw INTERFACE)
    target_link_libraries(LibRaw INTERFACE LibRaw::libraw)
endif()
