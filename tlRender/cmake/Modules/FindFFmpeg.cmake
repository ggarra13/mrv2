# Find the FFmpeg library.
#
# This module defines the following variables:
#
# * FFmpeg_INCLUDE_DIRS
# * FFmpeg_LIBRARIES
#
# This module defines the following imported targets:
#
# * FFmpeg::avcodec
# * FFmpeg::avdevice
# * FFmpeg::avformat
# * FFmpeg::avutil
# * FFmpeg::swresample
# * FFmpeg::swscale
#
# This module defines the following interfaces:
#
# * FFmpeg

find_path(FFmpeg_INCLUDE_DIR NAMES libavcodec/avcodec.h)
set(FFmpeg_INCLUDE_DIRS
    ${FFmpeg_INCLUDE_DIR})

find_library(FFmpeg_avcodec_LIBRARY NAMES avcodec)
find_library(FFmpeg_avdevice_LIBRARY NAMES avdevice)
find_library(FFmpeg_avformat_LIBRARY NAMES avformat)
find_library(FFmpeg_avutil_LIBRARY NAMES avutil)
find_library(FFmpeg_swresample_LIBRARY NAMES swresample)
find_library(FFmpeg_swscale_LIBRARY NAMES swscale)
set(FFmpeg_LIBRARIES
    ${FFmpeg_avformat_LIBRARY}
    ${FFmpeg_avcodec_LIBRARY}
    ${FFmpeg_swscale_LIBRARY}
    ${FFmpeg_avutil_LIBRARY}
    ${FFmpeg_swresample_LIBRARY}
    ${FFmpeg_avdevice_LIBRARY})
if(TLRENDER_NET)
    find_package(OpenSSL REQUIRED)
    list(APPEND FFmpeg_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR})
    list(APPEND FFmpeg_LIBRARIES ${OPENSSL_LIBRARIES})
endif()
if(APPLE)
    list(APPEND FFmpeg_LIBRARIES
        "-framework Security"
        "-framework VideoToolbox"
        "-framework CoreMedia"
        "-framework CoreVideo"
        "-framework CoreFoundation")
elseif(${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "arm")
    list(APPEND FFmpeg_LIBRARIES atomic)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    FFmpeg
    REQUIRED_VARS
        FFmpeg_INCLUDE_DIR
        FFmpeg_avcodec_LIBRARY
        FFmpeg_avdevice_LIBRARY
        FFmpeg_avformat_LIBRARY
        FFmpeg_avutil_LIBRARY
        FFmpeg_swresample_LIBRARY
        FFmpeg_swscale_LIBRARY)
mark_as_advanced(
    FFmpeg_INCLUDE_DIR
    FFmpeg_avcodec_LIBRARY
    FFmpeg_avdevice_LIBRARY
    FFmpeg_avformat_LIBRARY
    FFmpeg_avutil_LIBRARY
    FFmpeg_swresample_LIBRARY
    FFmpeg_swscale_LIBRARY)

if(FFmpeg_FOUND AND NOT TARGET FFmpeg::avutil)
    set(FFmpeg_avutil_INTERFACE_LINK_LIBRARIES)
    if(TLRENDER_NET)
        list(APPEND FFmpeg_avutil_INTERFACE_LINK_LIBRARIES OpenSSL)
    endif()
    add_library(FFmpeg::avutil UNKNOWN IMPORTED)
    set_target_properties(FFmpeg::avutil PROPERTIES
        IMPORTED_LOCATION "${FFmpeg_avutil_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS FFmpeg_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${FFmpeg_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${FFmpeg_avutil_INTERFACE_LINK_LIBRARIES}")
endif()
if(FFmpeg_FOUND AND NOT TARGET FFmpeg::swresample)
    add_library(FFmpeg::swresample UNKNOWN IMPORTED)
    set_target_properties(FFmpeg::swresample PROPERTIES
        IMPORTED_LOCATION "${FFmpeg_swresample_LIBRARY}"
        INTERFACE_LINK_LIBRARIES "FFmpeg::avutil")
endif()
if(FFmpeg_FOUND AND NOT TARGET FFmpeg::swscale)
    add_library(FFmpeg::swscale UNKNOWN IMPORTED)
    set_target_properties(FFmpeg::swscale PROPERTIES
        IMPORTED_LOCATION "${FFmpeg_swscale_LIBRARY}"
        INTERFACE_LINK_LIBRARIES "FFmpeg::avutil")
endif()
if(FFmpeg_FOUND AND NOT TARGET FFmpeg::avcodec)
    add_library(FFmpeg::avcodec UNKNOWN IMPORTED)
    set_target_properties(FFmpeg::avcodec PROPERTIES
        IMPORTED_LOCATION "${FFmpeg_avcodec_LIBRARY}"
        INTERFACE_LINK_LIBRARIES "FFmpeg::swresample;FFmpeg::avutil")
endif()
if(FFmpeg_FOUND AND NOT TARGET FFmpeg::avformat)
    add_library(FFmpeg::avformat UNKNOWN IMPORTED)
    set(FFmpeg_avformat_LINK_LIBRARIES FFmpeg::avcodec FFmpeg::swresample FFmpeg::avutil)
    if(${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "arm" AND NOT APPLE)
       list(APPEND FFmpeg_avformat_LINK_LIBRARIES atomic)
    endif()
    set_target_properties(FFmpeg::avformat PROPERTIES
        IMPORTED_LOCATION "${FFmpeg_avformat_LIBRARY}"
        INTERFACE_LINK_LIBRARIES "${FFmpeg_avformat_LINK_LIBRARIES}")
endif()
if(FFmpeg_FOUND AND NOT TARGET FFmpeg::avdevice)
    add_library(FFmpeg::avdevice UNKNOWN IMPORTED)
    set_target_properties(FFmpeg::avdevice PROPERTIES
        IMPORTED_LOCATION "${FFmpeg_avdevice_LIBRARY}"
        INTERFACE_LINK_LIBRARIES "FFmpeg::avformat;FFmpeg::avcodec;FFmpeg::swscale;FFmpeg::swresample;FFmpeg::avutil")
endif()
if(FFmpeg_FOUND AND NOT TARGET FFmpeg)
    add_library(FFmpeg INTERFACE)
    target_link_libraries(FFmpeg INTERFACE FFmpeg::avcodec)
    target_link_libraries(FFmpeg INTERFACE FFmpeg::avdevice)
    target_link_libraries(FFmpeg INTERFACE FFmpeg::avformat)
    target_link_libraries(FFmpeg INTERFACE FFmpeg::avutil)
    target_link_libraries(FFmpeg INTERFACE FFmpeg::swresample)
    target_link_libraries(FFmpeg INTERFACE FFmpeg::swscale)
endif()
