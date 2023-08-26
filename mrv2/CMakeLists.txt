cmake_minimum_required(VERSION 3.19)

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)


project(mrv2)

# For macOS, finds Frameworks last.  Uses Find*.cmake first.
set(CMAKE_FIND_FRAMEWORK LAST)


include( ../cmake/version.cmake )
include( ../cmake/options.cmake )
include( ../cmake/functions.cmake )

list(PREPEND CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}/../tlRender/cmake/Modules
    ${PROJECT_SOURCE_DIR}/../cmake/Modules)





#
# Add compile options
#
if ( UNIX )
    add_compile_options( -fvisibility=hidden )
endif()

# Make sure we pick our local FLTK copy
set( FLTK_INCLUDE_DIR ${CMAKE_PREFIX_PATH}/include )
find_package( FLTK REQUIRED CONFIG )
message( STATUS "Found FLTK: ${FLTK_INCLUDE_DIR}" )

find_package(Intl)
find_package(tlRender REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(GLFW REQUIRED)

if( MRV2_NETWORK )
    find_package(Poco REQUIRED Net Foundation)
    add_definitions( -DMRV2_NETWORK )
endif()

set(LibHaru )
if( MRV2_PDF )
    find_library( LibHaru NAMES hpdf )
    if(UNIX AND NOT APPLE)
	list(APPEND LibHaru m)
    endif()
    add_definitions( -DMRV2_PDF )
endif()

if(TLRENDER_USD)
    find_package(pxr)
    if(pxr_FOUND)
        add_definitions(-DTLRENDER_USD)
    endif()
endif()

#
# Make sure we use our own recently compiled fluid executable
#
if( WIN32 )
    set( FLTK_FLUID_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/fluid-cmd)
else()
    set( FLTK_FLUID_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/fluid)
endif()

if(TLRENDER_USD)
    add_definitions(-DTLRENDER_USD)
endif()

if(TLRENDER_BMD)
    add_definitions(-DTLRENDER_BMD)
endif()

if(TLRENDER_GL)
    add_definitions(-DTLRENDER_GL)
endif()


if(MRV2_PYBIND11)
    #
    # Search for our installed python executable
    #
    find_package(Python REQUIRED Interpreter Development)
    add_definitions(-DMRV2_PYBIND11)
    find_package(pybind11 REQUIRED)
endif()

# Omit superfluous "Up-to-date" messages.
if(NOT DEFINED CMAKE_INSTALL_MESSAGE)
    set(CMAKE_INSTALL_MESSAGE "LAZY")
endif()

#
# Add some defines
#
add_definitions(
    -D NOMINMAX
    -DMRV2_VERSION="${mrv2_VERSION}"
    -DTLRENDER_VERSION="${tlRender_VERSION}"
)


# Directory of generated .cxx/.h built from .fl files
set( mrv2_FLUID_INCLUDE_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/lib/mrvWidgets
    )

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions( -DTLRENDER_API_GL_4_1_Debug )
    set(tlRender_glad "${CMAKE_CURRENT_SOURCE_DIR}/../tlRender/deps/glad_GL_4_1_Debug")
else()
   set(tlRender_glad "${CMAKE_CURRENT_SOURCE_DIR}/../tlRender/deps/glad_GL_4_1")
endif()

set( MRV_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/lib   # for mrv2 includes (search here first)
    ${mrv2_FLUID_INCLUDE_DIR}         # for mrv2 .fl converted files next
    ${tlRender_glad}
    ${CMAKE_PREFIX_PATH}/include/OpenEXR
    ${CMAKE_PREFIX_PATH}/include/Imath
    ${CMAKE_PREFIX_PATH}/include/opentimelineio/deps
    ${CMAKE_PREFIX_PATH}/include/boost-1_70
    ${CMAKE_PREFIX_PATH}/include/tlRender
    ${CMAKE_PREFIX_PATH}/include
    ${tlRender_INCLUDE_DIRS}
    # We need to include all these non-public tlRender directories
    # as we get the version information from them as well as some variables
    # and constants.
    ${CMAKE_CURRENT_SOURCE_DIR}/../tlRender/deps
    ${CMAKE_CURRENT_SOURCE_DIR}/../tlRender/lib
    # For icons on the mrv2/icons directory.
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Intl_INCLUDE_DIRS}
)

include_directories( ${MRV_INCLUDE_DIRS} )

#
# Install mrv2's auxiliary directories
#
install( DIRECTORY colors docs fonts icons ocio python share
    DESTINATION .
    COMPONENT applications )


#
# These two variables will contain the sources for .po tranalation.
# PO_SOURCES will contain relative paths while PO_ABS_SOURCES will conttain
# the absolute paths.
#
set( PO_SOURCES )
set( PO_ABS_SOURCES )

#
# Add auxiliary libraries
#
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/lib )

#
# Add main source
#
add_subdirectory( src )

#
# Add the packaging logic
#
include( ../cmake/packaging.cmake )
