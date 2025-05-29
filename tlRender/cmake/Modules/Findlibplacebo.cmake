# Find the libplacebo library.
#
# This module defines the following variables:
#
# * libplacebo_FOUND
# * libplacebo_INCLUDE_DIRS
# * libplacebo_LIBRARIES
#
# This module defines the following imported targets:
#
# * libplacebo::libplacebo
#
# This module defines the following interfaces:
#
# * libplacebo

find_path(libplacebo_INCLUDE_DIR NAMES libplacebo/dummy.h)
set(libplacebo_INCLUDE_DIRS ${libplacebo_INCLUDE_DIR})

if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    find_library(libplacebo_LIBRARY
        NAMES placebod placebo libplacebod libplacebo)
else()
    find_library(libplacebo_LIBRARY
        NAMES placebo libplacebo)
endif()
set(libplacebo_LIBRARIES
    ${libplacebo_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    libplacebo
    REQUIRED_VARS libplacebo_INCLUDE_DIR libplacebo_LIBRARY)
mark_as_advanced(libplacebo_INCLUDE_DIR libplacebo_LIBRARY)

if(libplacebo_FOUND AND NOT TARGET libplacebo::libplacebo)
    add_library(libplacebo::libplacebo UNKNOWN IMPORTED)
    set_target_properties(libplacebo::libplacebo PROPERTIES
        IMPORTED_LOCATION "${libplacebo_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS libplacebo_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${libplacebo_INCLUDE_DIR}")
endif()
if(libplacebo_FOUND AND NOT TARGET libplacebo)
    add_library(libplacebo INTERFACE)
    target_link_libraries(libplacebo INTERFACE libplacebo::libplacebo)
endif()
