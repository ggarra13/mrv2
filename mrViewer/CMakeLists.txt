cmake_minimum_required(VERSION 3.19)

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)

project(mrViewer2)


# For macOS, finds Frameworks last.  Uses Find*.cmake first.
set(CMAKE_FIND_FRAMEWORK LAST)

list(PREPEND CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/../tlRender/cmake/Modules
  ${PROJECT_SOURCE_DIR}/../cmake/Modules )


include( ../cmake/options.cmake )

find_package( FLTK     REQUIRED CONFIG )

set( Boost_INCLUDEDIR ${CMAKE_PREFIX_PATH}/include )
set( Boost_LIBRARYDIR ${CMAKE_PREFIX_PATH}/lib )
set( Boost_NO_SYSTEM_PATHS TRUE )
find_package( Boost    REQUIRED COMPONENTS filesystem regex )

find_package( LibIntl  REQUIRED )

if( WIN32 )
    find_package( GLEW  REQUIRED )
endif()

set( FLTK_FLUID_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/fluid)


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
  ${GLEW_INCLUDE_DIR}
  ${LIBINTL_INCLUDE_DIR}
  /usr/local/include
  )
include_directories( ${include_dirs} )


add_definitions( -DUSE_GETTEXT )


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
  
# Python dependencies
if(TLRENDER_PYTHON)
    find_package(Python3 REQUIRED COMPONENTS Development)
endif()

#
# Install mrViewer's auxiliary directories
#
install( DIRECTORY colors ocio DESTINATION ${CMAKE_INSTALL_PREFIX} )


add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/lib )
add_subdirectory( src )
