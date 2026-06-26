# Find the nlohmann-json library.
#
# This module defines the following variables:
#
# * nlohmann-json_FOUND
# * nlohmann-json_INCLUDE_DIRS
#
# This module defines the following imported targets:
#
# * nlohmann-json::nlohmann-json
#
# This module defines the following interfaces:
#
# * nlohmann-json

find_path(nlohmann-json_INCLUDE_DIR NAMES nlohmann/json.hpp)
set(nlohmann-json_INCLUDE_DIRS ${nlohmann-json_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    nlohmann-json
    REQUIRED_VARS nlohmann-json_INCLUDE_DIR)
mark_as_advanced(nlohmann-json_INCLUDE_DIR)

if(nlohmann-json_FOUND AND NOT TARGET nlohmann-json::nlohmann-json)
    add_library(nlohmann-json::nlohmann-json UNKNOWN IMPORTED)
    set_target_properties(nlohmann-json::nlohmann-json PROPERTIES
        INTERFACE_COMPILE_DEFINITIONS nlohmann-json_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${nlohmann-json_INCLUDE_DIR}")
endif()
if(nlohmann-json_FOUND AND NOT TARGET nlohmann-json)
    add_library(nlohmann-json INTERFACE)
    target_link_libraries(nlohmann-json INTERFACE nlohmann-json::nlohmann-json)
endif()
