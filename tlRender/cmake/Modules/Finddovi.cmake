# Find the dovi library.
#
# This module defines the following variables:
#
# * dovi_INCLUDE_DIRS
# * dovi_LIBRARIES
#
# This module defines the following imported targets:
#
# * dovi::dovi
#

# Find dovi

find_path(dovi_INCLUDE_DIR
    NAMES libdovi/rpu_parser.h)

find_library(dovi_LIBRARY
    NAMES dovi)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(dovi
    REQUIRED_VARS
        dovi_INCLUDE_DIR
        dovi_LIBRARY)

mark_as_advanced(
    dovi_INCLUDE_DIR
    dovi_LIBRARY)

if(dovi_FOUND AND NOT TARGET dovi::dovi)
    add_library(dovi::dovi SHARED IMPORTED GLOBAL)
    # or STATIC if libdovi.a
    set_target_properties(dovi::dovi PROPERTIES
        IMPORTED_LOCATION "${dovi_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${dovi_INCLUDE_DIR}")
endif()

# Legacy variables (optional)
set(dovi_INCLUDE_DIRS "${dovi_INCLUDE_DIR}")
set(dovi_LIBRARIES dovi::dovi)
