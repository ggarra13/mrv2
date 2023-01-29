cmake_minimum_required(VERSION 3.19)

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)


project(mrv2)


# For macOS, finds Frameworks last.  Uses Find*.cmake first.
set(CMAKE_FIND_FRAMEWORK LAST)

list(PREPEND CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}/../tlRender/cmake/Modules
    ${PROJECT_SOURCE_DIR}/../cmake/Modules )

include( ../cmake/version.cmake )
include( ../cmake/options.cmake )
include( ../cmake/functions.cmake )

find_package( FLTK     REQUIRED CONFIG )
find_package( LibIntl  REQUIRED )  # For internationalization (not yet used)

#
# Make sure we pick our own local version of boos
#
set( Boost_INCLUDEDIR ${CMAKE_PREFIX_PATH}/include )
set( Boost_LIBRARYDIR ${CMAKE_PREFIX_PATH}/lib )
set( Boost_NO_SYSTEM_PATHS TRUE )
find_package( Boost    REQUIRED COMPONENTS filesystem regex )


#
# Make sure we use our own fluid executable
#
if( WIN32 )
    set( FLTK_FLUID_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/fluid-cmd)
else()
    set( FLTK_FLUID_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/fluid)
endif()

set( MRV_INCLUDE_DIRS
    ${CMAKE_PREFIX_PATH}/include/tlRender
    ${CMAKE_PREFIX_PATH}/include/OpenEXR
    ${CMAKE_PREFIX_PATH}/include/Imath
    ${CMAKE_PREFIX_PATH}/include/opentimelineio/deps
    ${CMAKE_PREFIX_PATH}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/lib/mrvWidgets # .cxx/.h built from .fl files
    ${CMAKE_CURRENT_SOURCE_DIR}/../tlRender/lib
    ${LIBINTL_INCLUDE_DIR}
    )
include_directories( ${MRV_INCLUDE_DIRS} )

#
# Add some defines
#
add_definitions(-D USE_GETTEXT -D NOMINMAX -DMRV2_VERSION="${mrv2_VERSION}")

find_package( FSeq     REQUIRED )
find_package( OCIO     REQUIRED )
find_package( OTIO     REQUIRED )
find_package( FreeType REQUIRED )
find_package( RtAudio  REQUIRED )
find_package( OpenGL REQUIRED )

# I/O dependencies
if(TLRENDER_JPEG)
    find_package(JPEG)
    if(JPEG_FOUND)
	add_definitions(-DTLRENDER_JPEG)
    endif()
endif()
if(TLRENDER_TIFF)
    find_package(TIFF)
    if(TIFF_FOUND)
	add_definitions(-DTLRENDER_TIFF)
    endif()
endif()
if(TLRENDER_PNG)
    find_package(PNG)
    if(PNG_FOUND)
	add_definitions(-DTLRENDER_PNG)
    endif()
endif()
if(TLRENDER_EXR)
    find_package(OpenEXR)
    if(OpenEXR_FOUND)
	add_definitions(-DTLRENDER_EXR)
    endif()
endif()
if(TLRENDER_FFMPEG)
    find_package(FFmpeg)
    if(FFmpeg_FOUND)
	add_definitions(-DTLRENDER_FFMPEG)
    endif()
endif()
if(TLRENDER_BMD)
    add_defiitions(-DTLRENDER_BMD)
endif()

# Python dependencies
if(TLRENDER_PYTHON)
    find_package(Python3 REQUIRED COMPONENTS Development)
endif()

# Omit superfluous "Up-to-date" messages.
if(NOT DEFINED CMAKE_INSTALL_MESSAGE)
    set(CMAKE_INSTALL_MESSAGE "LAZY")
endif()

#
# Install mrv2's auxiliary directories
#
install( DIRECTORY colors ocio icons docs shaders
    DESTINATION .
    COMPONENT applications )


#
# Add auxiliary libraries
#
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/lib )

#
# Add main source
#
add_subdirectory( src )

include( ../cmake/packaging.cmake )
