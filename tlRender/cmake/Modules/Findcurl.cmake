# Find the libpng library.
#
# This module defines the following variables:
#
# * curl_INCLUDE_DIRS
# * curl_LIBRARIES
#
# This module defines the following imported targets:
#
# * curl::curl
#
# This module defines the following interfaces:
#
# * curl

find_package(OpenSSL REQUIRED)
find_package(libssh2 REQUIRED)

find_path(curl_INCLUDE_DIR NAMES curl/curl.h)
set(curl_INCLUDE_DIRS
    ${curl_INCLUDE_DIR}
    ${OpenSSL_INCLUDE_DIRS}
    ${libssh2_INCLUDE_DIRS})

find_library(curl_LIBRARY
    NAMES curl libcurl
    HINTS ${CMAKE_INSTALL_PREFIX})
set(curl_LIBRARIES
    ${curl_LIBRARY}
    ${OpenSSL_LIBRARIES}
    ${libssh2_LIBRARIES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    curl
    REQUIRED_VARS curl_INCLUDE_DIR curl_LIBRARY)
mark_as_advanced(curl_INCLUDE_DIR curl_LIBRARY)

if(curl_FOUND AND NOT TARGET curl::libcurl)
    add_library(curl::libcurl UNKNOWN IMPORTED)
    set_target_properties(curl::libcurl PROPERTIES
        IMPORTED_LOCATION "${curl_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS curl_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${curl_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "OpenSSL;libssh2")
endif()
if(curl_FOUND AND NOT TARGET curl)
    add_library(curl INTERFACE)
    target_link_libraries(curl INTERFACE curl::libcurl)
endif()
