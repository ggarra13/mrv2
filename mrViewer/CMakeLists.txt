cmake_minimum_required(VERSION 3.19)

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)

project(mrViewer2)


# For macOS, finds Frameworks last.  Uses Find*.cmake first.
set(CMAKE_FIND_FRAMEWORK LAST)

list(PREPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../tlRender/cmake/Modules)


find_package( FLTK     REQUIRED CONFIG )
find_package( Boost    REQUIRED COMPONENTS filesystem )

find_program(FLTK_FLUID_EXECUTABLE
  NAME fluid
  HINTS ${CMAKE_INSTALL_PREFIX}/bin)


set( include_dirs
  ${CMAKE_PREFIX_PATH}/include/tlRender
  ${CMAKE_PREFIX_PATH}/include/OpenEXR
  ${CMAKE_PREFIX_PATH}/include/Imath
  ${CMAKE_PREFIX_PATH}/include/opentimelineio/deps
  ${CMAKE_PREFIX_PATH}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/lib/mrvFl
  ${CMAKE_CURRENT_SOURCE_DIR}/../tlRender/lib
  )
include_directories( ${include_dirs} )


add_definitions( -DUSE_GETTEXT )

find_package( FSeq     REQUIRED )
find_package( OCIO     REQUIRED )
find_package( OTIO     REQUIRED )
find_package( FreeType REQUIRED )
find_package( RtAudio  REQUIRED )
find_package(JPEG)
if(JPEG_FOUND)
    add_definitions(-DJPEG_FOUND)
endif()
find_package(TIFF)
if(TIFF_FOUND)
    add_definitions(-DTIFF_FOUND)
endif()
find_package(PNG)
if(PNG_FOUND)
    add_definitions(-DPNG_FOUND)
endif()
find_package(OpenEXR)
if(OpenEXR_FOUND)
    add_definitions(-DOpenEXR_FOUND)
endif()
find_package(FFmpeg)
if(FFmpeg_FOUND)
    add_definitions(-DFFmpeg_FOUND)
endif()
if(TLRENDER_ENABLE_PYTHON)
    find_package(Python3 REQUIRED COMPONENTS Development)
endif()

add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/lib )
add_subdirectory( src )
