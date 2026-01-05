cmake_minimum_required(VERSION 3.21)

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)


project(mrv2)

# For macOS, finds Frameworks last.  Uses Find*.cmake first.
set(CMAKE_FIND_FRAMEWORK LAST)

# Do not skip RPATH as that can make macOS crash.
set(CMAKE_SKIP_RPATH FALSE)

# Avoid warnings of overshadowing system libraries.
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "@executable_path/../lib;${CMAKE_INSTALL_PREFIX}/lib;/usr/local/lib")

# Build the executable with the final CMAKE_INSTALL_RPATH from the very beginning.
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)


include( ../cmake/version.cmake )
include( ../cmake/options.cmake )
include( ../cmake/functions.cmake )

list(PREPEND CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}/../tlRender/cmake/Modules
    ${PROJECT_SOURCE_DIR}/../cmake/Modules
    ${CMAKE_INSTALL_PREFIX}/lib/cmake)





#
# Add compile options
#
if ( UNIX )
    add_compile_options( -fvisibility=hidden )
    if (NOT APPLE)
	link_directories(${CMAKE_INSTALL_PREFIX}/lib64)
    endif()
    link_directories(${CMAKE_INSTALL_PREFIX}/lib)
endif()

if (MRV2_BACKEND STREQUAL "VK" OR MRV2_BACKEND STREQUAL "BOTH")
    find_package(Vulkan OPTIONAL_COMPONENTS glslang shaderc_combined SPIRV-Tools)
endif()

find_package(FLTK CONFIG REQUIRED)

set(FLTK_vk_LIBRARY )

if(FLTK_BUILD_SHARED_LIBS)
    if(MSVC)
	set(FLTK_LIBRARIES fltk::fltk-shared )
	set(FLTK_gl_LIBRARY )
	set(FLTK_vk_LIBRARY )
    else()
	set(FLTK_LIBRARIES fltk::fltk-shared fltk::images-shared )
	set(FLTK_gl_LIBRARY fltk::gl-shared)
	if(TLRENDER_VK)
	    set(FLTK_vk_LIBRARY fltk::vk-shared)
	endif()
    endif()
else()
    set(FLTK_LIBRARIES fltk::fltk fltk::images )
    set(FLTK_gl_LIBRARY fltk::gl )
    if(TLRENDER_VK)
	set(FLTK_vk_LIBRARY fltk::vk)
    endif()
endif()

#
# Make sure we use our own recently compiled fluid executable
#
if(MSVC)
    set(FLTK_FLUID_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/fluid-cmd.exe)
else()
    set(FLTK_FLUID_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/fluid)
endif()

message( STATUS "Found FLTK: ${FLTK_INCLUDE_DIR}" )
message( STATUS "FLTK LIBRARIES: ${FLTK_vk_LIBRARY} ${FLTK_gl_LIBRARY} ${FLTK_LIBRARIES}" )
message( STATUS "FLTK_FLUID_EXECUTABLE: ${FLTK_FLUID_EXECUTABLE}" )


if (UNIX)
    find_package(Intl REQUIRED)  # old method
else()
    if (WIN32)
	find_package(Iconv REQUIRED)
	find_package(Intl REQUIRED)
	set(Intl_LIBRARIES Iconv::Iconv Intl::Intl)
    else()
	find_package(Intl REQUIRED)  # old method
    endif()
endif()

find_package(tlRender REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(glfw3 REQUIRED)
find_package(minizip REQUIRED)
find_package(OpenGL REQUIRED)

# Now define a preprocessor macro
if(MRV2_BACKEND STREQUAL "GL")
    add_definitions(-DMRV2_BACKEND_GL=1)
elseif(MRV2_BACKEND STREQUAL "VK")
    add_definitions(-DMRV2_BACKEND_VK=1)
elseif(MRV2_BACKEND STREQUAL "BOTH")
    add_definitions(-DMRV2_BACKEND_BOTH=1)
endif()

if(MRV2_CPPTRACE)
    add_definitions( -D MRV2_CPPTRACE )
endif()

if(MRV2_NETWORK)
    find_package(Poco REQUIRED
	COMPONENTS Net Foundation
	CONFIG)
    add_definitions( -DMRV2_NETWORK )
endif()

if(MRV2_PDF)
    add_definitions( -DMRV2_PDF )
endif()

# Pass flags for CMAKE_BUILD_TYPE
if (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_definitions( -D MRV2_RelWithDebInfo )
endif()

if(NOT APPLE)
    if (MRV2_CPPTRACE)
	find_package(cpptrace CONFIG REQUIRED)
    endif()
endif()

if("${TLRENDER_API}" STREQUAL "GL_4_1_Debug")
    add_definitions(-DTLRENDER_API_GL_4_1_Debug)
endif()

if(TLRENDER_AOM)
    add_definitions(-DTLRENDER_AOM)
endif()

if(TLRENDER_AUDIO)
    add_definitions(-DTLRENDER_AUDIO)
endif()

if(TLRENDER_BMD)
    add_definitions(-DTLRENDER_BMD)
endif()

if(TLRENDER_EXR)
    add_definitions(-DTLRENDER_EXR)
endif()

if(TLRENDER_FFMPEG)
    add_definitions(-DTLRENDER_FFMPEG)
endif()

if(TLRENDER_GL)
    add_definitions(-DTLRENDER_GL)
endif()

if(TLRENDER_HAP)
    add_definitions(-DTLRENDER_HAP)
endif()

if(TLRENDER_JPEG)
    add_definitions(-DTLRENDER_JPEG)
endif()

if(TLRENDER_LIBPLACEBO)
    add_definitions(-DTLRENDER_LIBPLACEBO)
endif()

if(TLRENDER_NDI)
    add_definitions(-DTLRENDER_NDI)
    if (TLRENDER_NDI_SDK MATCHES ".*Advanced.*")
	add_definitions(-DTLRENDER_NDI_ADVANCED)
    endif()
endif()

if(TLRENDER_NET)
    add_definitions(-DTLRENDER_NET)
endif()

if(TLRENDER_OCIO)
    add_definitions(-DTLRENDER_OCIO)
endif()

if(TLRENDER_OPENJPH)
    add_definitions(-DTLRENDER_OPENJPH)
endif()

if(TLRENDER_PNG)
    add_definitions(-DTLRENDER_PNG)
endif()

if(TLRENDER_RAW)
    find_package(Jasper)
    if (JASPER_FOUND)
        add_definitions(-DTLRENDER_RAW)
    endif()
endif()

if(TLRENDER_SSL)
    add_definitions(-DTLRENDER_SSL)
endif()

if(TLRENDER_STB)
    add_definitions(-DTLRENDER_STB)
endif()

if(TLRENDER_SVTAV1)
    add_definitions(-DTLRENDER_SVTAV1)
endif()

if(TLRENDER_TIFF)
    add_definitions(-DTLRENDER_TIFF)
endif()

if(TLRENDER_USD)
    find_package(pxr)
    if(pxr_FOUND)
        add_definitions(-DTLRENDER_USD)
    endif()
endif()

if(TLRENDER_VK)
    add_definitions(-DTLRENDER_VK)
endif()

if(TLRENDER_WAYLAND)
    add_definitions(-DTLRENDER_WAYLAND -DFLTK_USE_WAYLAND)
endif()

if(TLRENDER_X11)
    add_definitions(-DTLRENDER_X11 -DFLTK_USE_X11)
endif()

if(TLRENDER_X264)
    add_definitions(-DTLRENDER_X264)
endif()


if(MRV2_PYBIND11 OR BUILD_PYTHON)
    find_package(Python REQUIRED COMPONENTS Interpreter Development)
endif()

if(MRV2_PYBIND11)
    add_definitions(-DMRV2_PYBIND11)
    find_package(pybind11 REQUIRED)
endif()

if(MRV2_PYFLTK)
    add_definitions(-DMRV2_PYFLTK)
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
    ${CMAKE_CURRENT_BINARY_DIR}/lib/mrvHDRWidgets
)

#
# We are currently not using boost anymore, but we might use it again if
# we turn on python's USD support.
#

set( mrv2_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/lib   # for mrv2 includes (search here first)
    ${mrv2_FLUID_INCLUDE_DIR}         # for mrv2 .fl converted files next
    ${CMAKE_PREFIX_PATH}/include/OpenEXR
    ${CMAKE_PREFIX_PATH}/include/Imath
    ${CMAKE_PREFIX_PATH}/include/opentimelineio/deps
    ${CMAKE_PREFIX_PATH}/include/tlRender
    ${CMAKE_PREFIX_PATH}/include
    ${tlRender_INCLUDE_DIRS}
    # We need to include all these non-public tlRender directories
    # as we get the version information from them as well as some variables
    # and constants.
    ${TLRENDER_NDI_SDK}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../tlRender/deps
    ${CMAKE_CURRENT_SOURCE_DIR}/../tlRender/lib
    # For icons on the mrv2/icons directory.
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Intl_INCLUDE_DIRS}
)

include_directories( ${mrv2_INCLUDE_DIRS} )

#
# Install mrv2's auxiliary application directories
#
install( DIRECTORY certs colors ocio presets
    DESTINATION .
    COMPONENT applications )

if (UNIX AND NOT APPLE)
    install( FILES icons/mrv2.png
	DESTINATION icons
	COMPONENT applications )
elseif(WIN32)
    install( FILES icons/mrv2.png
	DESTINATION icons
	COMPONENT applications )
endif()

#
# Install locales
#
install( DIRECTORY share/locale
    DESTINATION share
    COMPONENT applications)

#
# Install update-mrv2 plugin
#
if (MRV2_PYBIND11)
    install( FILES python/plug-ins/update-mrv2.py
	DESTINATION python/plug-ins
	COMPONENT applications )
    
    install( DIRECTORY ${CMAKE_INSTALL_PREFIX}/python/opentimelineio
	DESTINATION python/
	COMPONENT applications )
    
    #
    # Install mrv2's python demos
    #
    install( DIRECTORY python/demos
	DESTINATION python
	COMPONENT python_demos )

    install( DIRECTORY python/plug-ins/demos
	DESTINATION python/plug-ins
	COMPONENT python_demos )
endif()

#
# Install mrv2's documentation
#
install( DIRECTORY docs
    DESTINATION .
    COMPONENT documentation )

#
# These two variables will contain the sources for .po translation.
# PO_SOURCES will contain relative paths while PO_ABS_SOURCES will contain
# the absolute paths.
# This is needed as gettext/cmake would run into path length issues on some
# platforms.
#
set( PO_SOURCES )
set( PO_ABS_SOURCES )

#
# HDR PO sources
#
set( PO_HDR_SOURCES )
set( PO_HDR_ABS_SOURCES )

#
# Licenser helper PO sources
#
set( PO_LIC_SOURCES )
set( PO_LIC_ABS_SOURCES )

# Retrieve the current Git branch name
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE  # This removes any trailing newline characters
)
# Retrieve the short Git commit hash
execute_process(
    COMMAND git rev-parse --short HEAD
    OUTPUT_VARIABLE GIT_SHORT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Optionally, print the branch name to verify
message(STATUS "Current Git Branch: ${GIT_BRANCH}")

# Define a preprocessor macro for the Git branch name and short hash
add_compile_definitions(GIT_BRANCH_NAME="${GIT_BRANCH}")
add_compile_definitions(GIT_SHORT_HASH="${GIT_SHORT_HASH}")

#
# Use UTF-8 with Microsoft's compiler
#
if(MSVC)
    add_compile_options(/utf-8)
endif()

#
# Add auxiliary libraries
#
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/lib )

#
# Add license_helper for licensing
#
add_subdirectory(license_helper)

#
# Add hdr source
#
if (TLRENDER_NDI AND TLRENDER_VK AND MRV2_HDR)
    add_subdirectory(hdr)
endif()

#
# Add main source
#
add_subdirectory(main)

if (APPLE)
    add_subdirectory(launcher)
endif()

if (MRV2_BACKEND STREQUAL "VK" OR MRV2_BACKEND STREQUAL "BOTH")
    add_subdirectory(tests)
endif()

#
# Add the packaging logic
#
include( ../cmake/packaging.cmake )
