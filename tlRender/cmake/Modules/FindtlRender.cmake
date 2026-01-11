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
# * tlRender::tlIO
# * tlRender::tlTimeline
# * tlRender::tlTimelineGL
# * tlRender::tlTimelineUI
# * tlRender::tlDevice
# * tlRender::tlDraw
# * tlRender::tlGL
# * tlRender::tlTimelineVk
# * tlRender::tlTimelineUIVk
# * tlRender::tlUI
# * tlRender::tlVk
# * tlRender::glad
#
# This module defines the following interfaces:
#
# * tlRender

set(tlRender_VERSION 0.0.1)

#
# Use this variable to debug where libraries are found
#
#set(CMAKE_FIND_DEBUG_MODE TRUE)

find_package(Imath REQUIRED CONFIG)
find_package(nlohmann_json REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenTimelineIO REQUIRED CONFIG)
find_package(PNG REQUIRED)
find_package(glfw3 REQUIRED)
find_package(RtAudio)

#
# These may be installed in cmake or not installed if the setting is off
#
if(TLRENDER_OCIO)
    find_package(OpenColorIO REQUIRED CONFIG)
endif()
if(TLRENDER_EXR)
    find_package(OpenEXR CONFIG)
endif()
if(TLRENDER_FFMPEG)
    find_package(FFmpeg)
endif()
if(TLRENDER_LIBPLACEBO)
    find_package(libplacebo)
endif()
if(TLRENDER_JPEG)
    find_package(libjpeg-turbo CONFIG)
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
if(TLRENDER_USD)
    find_package(pxr)
endif()

# set(CMAKE_FIND_DEBUG_MODE FALSE)


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
if(pxr_FOUND)
    list(APPEND tlRender_INCLUDE_DIRS ${pxr_INCLUDE_DIRS})
endif()

if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    find_library(tlRender_tlCore_LIBRARY NAMES tlCore)
    find_library(tlRender_tlDraw_LIBRARY NAMES tlDraw)
    find_library(tlRender_tlIO_LIBRARY NAMES tlIO)
    find_library(tlRender_tlTimeline_LIBRARY NAMES tlTimeline)
    find_library(tlRender_tlTimelineUI_LIBRARY NAMES tlTimelineUI)
    find_library(tlRender_tlDevice_LIBRARY NAMES tlDevice)
    find_library(tlRender_tlGL_LIBRARY NAMES tlGL)
    find_library(tlRender_tlTimelineGL_LIBRARY NAMES tlTimelineGL)
    find_library(tlRender_tlVk_LIBRARY NAMES tlVk)
    find_library(tlRender_tlUI_LIBRARY NAMES tlUI)
    find_library(tlRender_tlTimelineVk_LIBRARY NAMES tlTimelineVk)
    find_library(tlRender_tlTimelineUIVk_LIBRARY NAMES tlTimelineUIVk)
    find_library(tlRender_glad_LIBRARY NAMES glad)
else()
    find_library(tlRender_tlCore_LIBRARY NAMES tlCore)
    find_library(tlRender_tlDraw_LIBRARY NAMES tlDraw)
    find_library(tlRender_tlIO_LIBRARY NAMES tlIO)
    find_library(tlRender_tlTimeline_LIBRARY NAMES tlTimeline)
    find_library(tlRender_tlTimelineUI_LIBRARY NAMES tlTimelineUI)
    find_library(tlRender_tlDevice_LIBRARY NAMES tlDevice)
    find_library(tlRender_tlGL_LIBRARY NAMES tlGL)
    find_library(tlRender_tlTimelineGL_LIBRARY NAMES tlTimelineGL)
    find_library(tlRender_tlVk_LIBRARY NAMES tlVk)
    find_library(tlRender_tlUI_LIBRARY NAMES tlUI)
    find_library(tlRender_tlTimelineVk_LIBRARY NAMES tlTimelineVk)
    find_library(tlRender_tlTimelineUIVk_LIBRARY NAMES tlTimelineUIVk)
    find_library(tlRender_glad_LIBRARY NAMES glad)
endif()

set(tlRender_GL_FOUND FALSE)
set(tlRender_GL_LIBRARIES)
if(tlRender_tlGL_LIBRARY AND tlRender_tlTimelineGL_LIBRARY)
    set(tlRender_GL_FOUND TRUE)
    set(tlRender_GL_LIBRARIES
	${tlRender_tlTimelineUI_LIBRARY}
	${tlRender_tlTimelineGL_LIBRARY}
	${tlRender_tlGL_LIBRARY}
	${tlRender_glad_LIBRARY})
endif()

set(tlRender_VK_FOUND FALSE)
set(tlRender_VK_LIBRARIES)
if(tlRender_tlVk_LIBRARY AND tlRender_tlTimelineVk_LIBRARY)
    set(tlRender_VK_FOUND TRUE)
    set(tlRender_VK_LIBRARIES
	${tlRender_tlVk_LIBRARY}
	${tlRender_tlTimelineVk_LIBRARY}
	${tlRender_tlTimelineUIVk_LIBRARY})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    tlRender
    REQUIRED_VARS
        tlRender_INCLUDE_DIR
        tlRender_tlCore_LIBRARY
        tlRender_tlIO_LIBRARY
        tlRender_tlTimeline_LIBRARY)
mark_as_advanced(
    tlRender_INCLUDE_DIR
    tlRender_tlCore_LIBRARY
    tlRender_tlIO_LIBRARY
    tlRender_tlTimeline_LIBRARY
    tlRender_tlTimelineGL_LIBRARY
    tlRender_tlTimelineUI_LIBRARY
    tlRender_tlDraw_LIBRARY
    tlRender_tlDevice_LIBRARY
    tlRender_tlGL_LIBRARY
    tlRender_tlTimelineVk_LIBRARY
    tlRender_tlTimelineUIVk_LIBRARY
    tlRender_tlUI_LIBRARY
    tlRender_tlVk_LIBRARY
    tlRender_glad_LIBRARY)

set(tlRender_tlCore_LIBRARIES "OTIO::opentimelineio;Imath::Imath;nlohmann_json::nlohmann_json" )
set(tlRender_tlDraw_LIBRARIES "tlRender::tlCore")
if (OpenColorIO_FOUND)
    list(APPEND tlRender_tlCore_LIBRARIES OpenColorIO::OpenColorIO)
    list(APPEND tlRender_tlTimelineGL_LIBRARIES "tlRender::tlTimeline;tlRender::tlGL;OpenColorIO::OpenColorIO")
    list(APPEND tlRender_tlTimelineVk_LIBRARIES "tlRender::tlTimeline;tlRender::tlVk;OpenColorIO::OpenColorIO")
endif()
if (RtAudio_FOUND)
    list(APPEND tlRender_tlCore_LIBRARIES RtAudio)
endif()
if (libjpeg-turbo_FOUND)
    set(tlRender_tlIO_LIBRARIES libjpeg-turbo::turbojpeg-static )
endif()
if (libplacebo_FOUND)
    list(APPEND tlRender_tlTimelineGL_LIBRARIES "libplacebo::libplacebo")
    list(APPEND tlRender_tlTimelineVk_LIBRARIES "libplacebo::libplacebo")
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
if (glfw3_FOUND)
    list(APPEND tlRender_tlIO_LIBRARIES glfw3)
endif()
list(APPEND tlRender_tlIO_LIBRARIES tlRender::tlCore)
if (tlRender_GL_FOUND AND MRV2_BACKEND STREQUAL "GL")
    list(APPEND tlRender_tlIO_LIBRARIES tlRender::tlGL)
endif()
if (tlRender_Vk_FOUND)
    list(APPEND tlRender_tlIO_LIBRARIES tlRender::tlVk)
endif()
if (pxr_FOUND)
    list(APPEND tlRender_tlIO_LIBRARIES tf usd usdGeom)
endif()
if (tlRender_GL_FOUND AND pxr_FOUND)
    list(APPEND tlRender_tlIO_LIBRARIES usdImagingGL)
endif()
if (pxr_FOUND)
    list(APPEND tlRender_tlIO_LIBRARIES usdAppUtils hd)
endif()

set(tlRender_COMPILE_DEFINITIONS tlRender_FOUND)

if(tlRender_FOUND AND NOT TARGET tlRender::glad)
    add_library(tlRender::glad UNKNOWN IMPORTED)
    set_target_properties(tlRender::glad PROPERTIES
        IMPORTED_LOCATION "${tlRender_glad_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlCore)
    add_library(tlRender::tlCore UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlCore PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlCore_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${tlRender_tlCore_LIBRARIES}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlDraw)
    add_library(tlRender::tlDraw UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlDraw PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlDraw_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${tlRender_tlDraw_LIBRARIES}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlIO)
    add_library(tlRender::tlIO UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlIO PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlIO_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${tlRender_tlIO_LIBRARIES}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlTimeline)
    set(tlRender_tlTimeline_LIBRARIES "tlRender::tlIO")
    add_library(tlRender::tlTimeline UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlTimeline PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlTimeline_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${tlRender_tlTimeline_LIBRARIES}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlUI)
    add_library(tlRender::tlUI UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlUI PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlUI_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlTimelineUI)
    add_library(tlRender::tlTimelineUI UNKNOWN IMPORTED)
    set(tlRender_tlTimelineUI_LIBRARIES "tlRender::tlIO;tlRender::tlTimeline;tlRender::tlUI")
    set_target_properties(tlRender::tlTimelineUI PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlTimelineUI_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${tlRender_tlTimelineUI_LIBRARIES}")
endif()
if (tlRender_GL_FOUND)
    if(tlRender_FOUND AND NOT TARGET tlRender::tlGL)
	set(tlRender_tlGL_LIBRARIES "tlRender::tlCore;tlRender::glad;glfw3")
	add_library(tlRender::tlGL UNKNOWN IMPORTED)
	set_target_properties(tlRender::tlGL PROPERTIES
            IMPORTED_LOCATION "${tlRender_tlGL_LIBRARY}"
            INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
	    INTERFACE_LINK_LIBRARIES "${tlRender_tlGL_LIBRARIES}")
    endif()
    if(tlRender_FOUND AND NOT TARGET tlRender::tlTimelineGL)
	add_library(tlRender::tlTimelineGL UNKNOWN IMPORTED)
	set_target_properties(tlRender::tlTimelineGL PROPERTIES
            IMPORTED_LOCATION "${tlRender_tlTimelineGL_LIBRARY}"
            INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${tlRender_tlTimelineGL_LIBRARIES}")
    endif()
    if(tlRender_FOUND AND NOT TARGET tlRender::tlDevice)
	add_library(tlRender::tlDevice UNKNOWN IMPORTED)
	set(tlRender_tlDevice_LIBRARIES "tlRender::tlIO;tlRender::tlTimeline;")
	list(APPEND tlRender_tlDevice_LIBRARIES tlRender::tlGL)
	list(APPEND tlRender_tlDevice_LIBRARIES tlRender::tlTimelineGL)
	set_target_properties(tlRender::tlDevice PROPERTIES
            IMPORTED_LOCATION "${tlRender_tlDevice_LIBRARY}"
            INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
	    INTERFACE_LINK_LIBRARIES "${tlRender_tlDevice_LIBRARIES}")
    endif()
endif()

if (tlRender_VK_FOUND)
    if(tlRender_FOUND AND TLRENDER_VK AND NOT TARGET tlRender::tlVk)
	set(tlRender_tlVk_LIBRARIES "tlRender::tlCore")
	add_library(tlRender::tlVk UNKNOWN IMPORTED)
	set_target_properties(tlRender::tlVk PROPERTIES
            IMPORTED_LOCATION "${tlRender_tlVk_LIBRARY}"
            INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
	    INTERFACE_LINK_LIBRARIES "${tlRender_tlVk_LIBRARIES}")
    endif()
    if(tlRender_FOUND AND TLRENDER_VK AND NOT TARGET tlRender::tlTimelineVk)
	set(tlRender_tlTimelineVk_LIBRARIES "tlRender::tlVk;tlRender::tlTimeline")
	add_library(tlRender::tlTimelineVk UNKNOWN IMPORTED)
	set_target_properties(tlRender::tlTimelineVk PROPERTIES
            IMPORTED_LOCATION "${tlRender_tlTimelineVk_LIBRARY}"
            INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${tlRender_tlTimelineVk_LIBRARIES}")
    endif()
    if(tlRender_FOUND AND NOT TARGET tlRender::tlTimelineUIVk)
	set(tlRender_tlTimelineUIVk_LIBRARIES "tlRender::tlIO;tlRender::tlTimeline;tlRender::tlUI")
	add_library(tlRender::tlTimelineUIVk UNKNOWN IMPORTED)
	set_target_properties(tlRender::tlTimelineUIVk PROPERTIES
            IMPORTED_LOCATION "${tlRender_tlTimelineUIVk_LIBRARY}"
            INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${tlRender_tlTimelineUIVk_LIBRARIES}")
    endif()
    if(tlRender_FOUND AND NOT TARGET tlRender::tlDevice)
	add_library(tlRender::tlDevice UNKNOWN IMPORTED)
	set(tlRender_tlDevice_LIBRARIES "tlRender::tlIO;tlRender::tlTimeline")
	list(APPEND tlRender_tlDevice_LIBRARIES tlRender::tlVk)
	list(APPEND tlRender_tlDevice_LIBRARIES tlRender::tlTimelineVk)
	set_target_properties(tlRender::tlDevice PROPERTIES
            IMPORTED_LOCATION "${tlRender_tlDevice_LIBRARY}"
            INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
	    INTERFACE_LINK_LIBRARIES "${tlRender_tlDevice_LIBRARIES}")
    endif()
endif()

if(tlRender_FOUND AND NOT TARGET tlRender)
    add_library(tlRender INTERFACE)
    target_link_libraries(tlRender INTERFACE tlRender::tlCore)
    target_link_libraries(tlRender INTERFACE tlRender::tlIO)
    target_link_libraries(tlRender INTERFACE tlRender::tlTimeline)
    if (tlRender_tlTimelineUI_LIBRARY)
	target_link_libraries(tlRender INTERFACE tlRender::tlTimelineUI)
    endif()
    if (tlRender_GL_FOUND)
	target_link_libraries(tlRender INTERFACE tlRender::tlTimelineGL)
	target_link_libraries(tlRender INTERFACE tlRender::tlGL)
    endif()
    if (tlRender_tlTimelineUIVk_LIBRARY)
	target_link_libraries(tlRender INTERFACE tlRender::tlTimelineUIVk)
    endif()
    if (tlRender_VK_FOUND)
	target_link_libraries(tlRender INTERFACE tlRender::tlTimelineVk)
	target_link_libraries(tlRender INTERFACE tlRender::tlVk)
    endif()
    target_link_libraries(tlRender INTERFACE tlRender::tlDevice)
    target_link_libraries(tlRender INTERFACE tlRender::glad)
endif()
