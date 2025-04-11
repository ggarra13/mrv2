# Find the tlRender library.
#
# This module defines the following variables:
#
# * tlRender_FOUND
# * tlRender_VERSION
# * tlRender_INCLUDE_DIRS
# * tlRender_LIBRARIES
#
# This module defines the following imported targets:
#
# * tlRender::tlCore
# * tlRender::tlBaseApp
# * tlRender::tlIO
# * tlRender::tlTimeline
# * tlRender::tlTimelineGL
# * tlRender::tlTimelineUI
# * tlRender::tlDevice
# * tlRender::tlGL
# * tlRender::glad
#
# This module defines the following interfaces:
#
# * tlRender

set(tlRender_VERSION 0.0.1)

find_package(Imath REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Freetype REQUIRED)
find_package(OTIO REQUIRED)
find_package(PNG REQUIRED)
find_package(glfw3 REQUIRED)
find_package(RtAudio)

#
# These may be installed in cmake or not installed if the setting is off
#
if(TLRENDER_OCIO)
    find_package(OpenColorIO REQUIRED)
endif()
if(TLRENDER_EXR)
    find_package(OpenEXR)
endif()
if(TLRENDER_FFMPEG)
    find_package(FFmpeg)
endif()
if(TLRENDER_LIBPLACEBO)
    find_package(libplacebo)
endif()
if(TLRENDER_JPEG)
    find_package(libjpeg-turbo)
endif()
if(TLRENDER_NDI)
    find_package(NDI)
endif()
if(TLRENDER_RAW)
    find_package(LibRaw)
endif()
if(TLRENDER_TIFF)
    find_package(TIFF)
endif()

find_path(tlRender_INCLUDE_DIR NAMES tlCore/Util.h PATH_SUFFIXES tlRender)
set(tlRender_INCLUDE_DIRS
    ${tlRender_INCLUDE_DIR}
    ${Imath_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${OTIO_INCLUDE_DIRS}
    ${glfw3_INCLUDE_DIRS})

if(FFmpeg_FOUND)
    list(APPEND tlRender_INCLUDE_DIRS ${FFmpeg_INCLUDE_DIRS})
endif()
if(libjpeg-turbo_FOUND)
    list(APPEND tlRender_INCLUDE_DIRS ${libjpeg-turbo_INCLUDE_DIRS})
endif()
if(LibRaw_FOUND)
    list(APPEND tlRender_INCLUDE_DIRS ${LibRaw_INCLUDE_DIRS})
endif()
if(libplacebo_FOUND)
    list(APPEND tlRender_INCLUDE_DIRS ${libplacebo_INCLUDE_DIRS})
endif()
if(NDI_FOUND)
    list(APPEND tlRender_INCLUDE_DIRS ${NDI_INCLUDE_DIRS})
endif()
if(OpenEXR_FOUND)
    list(APPEND tlRender_INCLUDE_DIRS ${OpenEXR_INCLUDE_DIRS})
endif()
if(PNG_FOUND)
    list(APPEND tlRender_INCLUDE_DIRS ${PNG_INCLUDE_DIRS})
endif()
if(RtAudio_FOUND)
    list(APPEND tlRender_INCLUDE_DIRS ${RtAudio_INCLUDE_DIRS})
endif()
if(TIFF_FOUND)
    list(APPEND tlRender_INCLUDE_DIRS ${TIFF_INCLUDE_DIRS})
endif()

if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    find_library(tlRender_tlCore_LIBRARY NAMES tlCore)
    find_library(tlRender_tlBaseApp_LIBRARY NAMES tlBaseApp)
    find_library(tlRender_tlIO_LIBRARY NAMES tlIO)
    find_library(tlRender_tlTimeline_LIBRARY NAMES tlTimeline)
    find_library(tlRender_tlTimelineUI_LIBRARY NAMES tlTimelineUI)
    find_library(tlRender_tlDevice_LIBRARY NAMES tlDevice)
    find_library(tlRender_tlGL_LIBRARY NAMES tlGL)
    find_library(tlRender_tlTimelineGL_LIBRARY NAMES tlTimelineGL)
    find_library(tlRender_glad_LIBRARY NAMES glad)
else()
    find_library(tlRender_tlCore_LIBRARY NAMES tlCore)
    find_library(tlRender_tlBaseApp_LIBRARY NAMES tlBaseApp)
    find_library(tlRender_tlIO_LIBRARY NAMES tlIO)
    find_library(tlRender_tlTimeline_LIBRARY NAMES tlTimeline)
    find_library(tlRender_tlTimelineUI_LIBRARY NAMES tlTimelineUI)
    find_library(tlRender_tlDevice_LIBRARY NAMES tlDevice)
    find_library(tlRender_tlGL_LIBRARY NAMES tlGL)
    find_library(tlRender_tlTimelineGL_LIBRARY NAMES tlTimelineGL)
    find_library(tlRender_glad_LIBRARY NAMES glad)
endif()

set(tlRender_LIBRARIES
    ${tlRender_tlCore_LIBRARY}
    ${tlRender_tlIO_LIBRARY}
    ${tlRender_tlBaseApp_LIBRARY}
    ${tlRender_tlTimelineUI_LIBRARY}
    ${tlRender_tlTimeline_LIBRARY}
    ${tlRender_tlDevice_LIBRARY}
    ${tlRender_tlTimelineGL_LIBRARY}
    ${tlRender_tlGL_LIBRARY}
    ${tlRender_glad_LIBRARY}
    ${Imath_LIBRARIES}
    ${nlohmann_json_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${LIBPLACEBO_LIBRARIES}
    ${OTIO_LIBRARIES}
    ${RtAudio_LIBRARIES}
    ${TIFF_LIBRARIES}
    ${PNG_LIBRARIES}
    ${OpenEXR_LIBRARIES}
    ${FFmpeg_LIBRARIES}
    ${LibRaw_LIBRARIES}
    ${NDI_LIBRARIES}
    ${glfw3_LIBRARIES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    tlRender
    REQUIRED_VARS
        tlRender_INCLUDE_DIR
        tlRender_tlCore_LIBRARY
        tlRender_tlBaseApp_LIBRARY
        tlRender_tlIO_LIBRARY
        tlRender_tlTimeline_LIBRARY
        tlRender_tlTimelineGL_LIBRARY
        tlRender_tlTimelineUI_LIBRARY
        tlRender_tlDevice_LIBRARY
        tlRender_tlGL_LIBRARY
        tlRender_glad_LIBRARY)
mark_as_advanced(
    tlRender_INCLUDE_DIR
    tlRender_tlCore_LIBRARY
    tlRender_tlIO_LIBRARY
    tlRender_tlBaseApp_LIBRARY
    tlRender_tlTimeline_LIBRARY
    tlRender_tlTimelineGL_LIBRARY
    tlRender_tlTimelineUI_LIBRARY
    tlRender_tlDevice_LIBRARY
    tlRender_tlGL_LIBRARY
    tlRender_glad_LIBRARY)

set(tlRender_tlCore_LIBRARIES "OTIO;Imath::Imath;Freetype::Freetype;nlohmann_json::nlohmann_json" )
if (OpenColorIO_FOUND)
    list(APPEND tlRender_tlCore_LIBRARIES OpenColorIO::OpenColorIO)
    list(APPEND tlRender_tlTimelineGL_LIBRARIES "tlRender::tlTimeline;OpenColorIO::OpenColorIO")
endif()
if (RtAudio_FOUND)
    list(APPEND tlRender_tlCore_LIBRARIES RtAudio)
endif()
if (libjpeg-turbo_FOUND)
    set(tlRender_tlIO_LIBRARIES libjpeg-turbo::turbojpeg-static )
endif()
if (libplacebo_FOUND)
    list(APPEND tlRender_tlTimelineGL_LIBRARIES "libplacebo::libplacebo")
endif()
if (PNG_FOUND)
    list(APPEND tlRender_tlIO_LIBRARIES PNG)
endif()
if (TIFF_FOUND)
    list(APPEND tlRender_tlIO_LIBRARIES TIFF)
endif()
if (OpenEXR_FOUND)
    list(APPEND tlRender_tlIO_LIBRARIES OpenEXR::OpenEXR)
endif()
if (FFmpeg_FOUND)
    list(APPEND tlRender_tlIO_LIBRARIES FFmpeg)
endif()
if (LibRaw_FOUND)
    list(APPEND tlRender_tlIO_LIBRARIES LibRaw)
endif()
if (NDI_FOUND)
    list(APPEND tlRender_tlIO_LIBRARIES NDI)
endif()
list(APPEND tlRender_tlIO_LIBRARIES tlRender::tlCore)

set(tlRender_COMPILE_DEFINITIONS tlRender_FOUND)

if(tlRender_FOUND AND NOT TARGET tlRender::tlCore)
    add_library(tlRender::tlCore UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlCore PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlCore_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${tlRender_tlCore_LIBRARIES}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlIO)
    add_library(tlRender::tlIO UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlIO PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlIO_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${tlRender_tlIO_LIBRARIES}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlBaseApp)
    add_library(tlRender::tlBaseApp UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlBaseApp PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlBaseApp_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${tlRender_tlBaseApp_LIBRARIES}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlTimeline)
    add_library(tlRender::tlTimeline UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlTimeline PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlTimeline_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${tlRender_tTimeline_LIBRARIES}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlTimelineUI)
    add_library(tlRender::tlTimelineUI UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlTimelineUI PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlTimelineUI_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${tlRender_tTimelineUI_LIBRARIES}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlDevice)
    add_library(tlRender::tlDevice UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlDevice PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlDevice_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlGL)
    add_library(tlRender::tlGL UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlGL PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlGL_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlTimelineGL)
    add_library(tlRender::tlTimelineGL UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlTimelineGL PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlTimelineGL_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${tlRender_tlTimelineGL_LIBRARIES}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::glad)
    add_library(tlRender::glad UNKNOWN IMPORTED)
    set_target_properties(tlRender::glad PROPERTIES
        IMPORTED_LOCATION "${tlRender_glad_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender)
    add_library(tlRender INTERFACE)
    target_link_libraries(tlRender INTERFACE tlRender::tlCore)
    target_link_libraries(tlRender INTERFACE tlRender::tlIO)
    target_link_libraries(tlRender INTERFACE tlRender::tlBaseApp)
    target_link_libraries(tlRender INTERFACE tlRender::tlTimeline)
    target_link_libraries(tlRender INTERFACE tlRender::tlTimelineGL)
    target_link_libraries(tlRender INTERFACE tlRender::tlTimelineUI)
    target_link_libraries(tlRender INTERFACE tlRender::tlDevice)
    target_link_libraries(tlRender INTERFACE tlRender::tlGL)
    target_link_libraries(tlRender INTERFACE tlRender::glad)
endif()
