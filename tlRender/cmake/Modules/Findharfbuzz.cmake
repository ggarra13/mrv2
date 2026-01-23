# Find the harfbuzz library.
#
# This module defines the following variables:
#
# * harfbuzz_FOUND
# * harfbuzz_INCLUDE_DIRS
# * harfbuzz_LIBRARIES
#
# This module defines the following imported targets:
#
# * harfbuzz::harfbuzz
#
# This module defines the following interfaces:
#
# * harfbuzz

find_path(harfbuzz_INCLUDE_DIR NAMES harfbuzz/hb.h)
set(harfbuzz_INCLUDE_DIRS ${harfbuzz_INCLUDE_DIR})

if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    find_library(harfbuzz_LIBRARY
        NAMES harfbuzzd harfbuzz
	HINTS
	${CMAKE_INSTALL_PREFIX}/lib
    )
else()
    find_library(harfbuzz_LIBRARY
        NAMES harfbuzz
	HINTS
	${CMAKE_INSTALL_PREFIX}/lib)
endif()
set(harfbuzz_LIBRARIES
    ${harfbuzz_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    harfbuzz
    REQUIRED_VARS
    harfbuzz_INCLUDE_DIR
    harfbuzz_LIBRARY
)

if(harfbuzz_FOUND AND NOT TARGET harfbuzz::harfbuzz)
    add_library(harfbuzz::harfbuzz UNKNOWN IMPORTED)
    set_target_properties(harfbuzz::harfbuzz PROPERTIES
        IMPORTED_LOCATION "${harfbuzz_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS harfbuzz_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${harfbuzz_INCLUDE_DIR}")
endif()
if(harfbuzz_FOUND AND NOT TARGET harfbuzz)
    add_library(harfbuzz INTERFACE)
    target_link_libraries(harfbuzz INTERFACE harfbuzz::harfbuzz)
endif()
