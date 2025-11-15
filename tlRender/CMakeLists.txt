cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0091 NEW)  # Do not place MSVC's runtime flags 
cmake_policy(SET CMP0114 NEW)  # ExternalProject_Add steps improvement
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW) # Sets URL timestamps extraction
endif()

set(TLRENDER_VERSION_MAJOR "0")
set(TLRENDER_VERSION_MINOR "1")
set(TLRENDER_VERSION_PATCH "0")
set(TLRENDER_VERSION ${TLRENDER_VERSION_MAJOR}.${TLRENDER_VERSION_MINOR}.${TLRENDER_VERSION_PATCH})
add_definitions(-DTLRENDER_VERSION_MAJOR=${TLRENDER_VERSION_MAJOR})
add_definitions(-DTLRENDER_VERSION_MINOR=${TLRENDER_VERSION_MINOR})
add_definitions(-DTLRENDER_VERSION_PATCH=${TLRENDER_VERSION_PATCH})
add_definitions(-DTLRENDER_VERSION="${TLRENDER_VERSION}")

project(
    tlRender
    VERSION ${TLRENDER_VERSION}
    DESCRIPTION "tlRender is an open source library for building playback and review applications for visual effects, film, and animation"
    HOMEPAGE_URL "https://github.com/darbyjohnston/tlRender"
    LANGUAGES CXX C)

#-------------------------------------------------------------------------------
# Build options

set(TLRENDER_PYTHON FALSE CACHE BOOL "Enable Python support (for OTIO Python adapters)")
set(TLRENDER_API "GL_4_1" CACHE STRING "Graphics API (GL_4_1, GL_4_1_Debug, GLES_2)")
set(TLRENDER_GLFW TRUE CACHE BOOL "Enable support for GLFW")
set(TLRENDER_NET FALSE CACHE BOOL "Enable network support")
set(TLRENDER_OCIO TRUE CACHE BOOL "Enable support for OpenColorIO")
set(TLRENDER_AUDIO TRUE CACHE BOOL "Enable support for audio")
set(TLRENDER_JPEG TRUE CACHE BOOL "Enable support for JPEG I/O")
set(TLRENDER_TIFF TRUE CACHE BOOL "Enable support for TIFF I/O")
set(TLRENDER_STB TRUE CACHE BOOL "Enable support for STB I/O (TGA, BMP, PSD)")
set(TLRENDER_LIBPLACEBO FALSE CACHE BOOL "Enable support for libplacebo")
set(TLRENDER_PNG TRUE CACHE BOOL "Enable support for PNG I/O")
set(TLRENDER_EXR TRUE CACHE BOOL "Enable support for EXR I/O")
set(TLRENDER_FFMPEG TRUE CACHE BOOL "Enable support for FFmpeg I/O")
set(TLRENDER_USD FALSE CACHE BOOL "Enable support for USD")
set(TLRENDER_RAW TRUE CACHE BOOL  "Enable support for LibRaw")
set(TLRENDER_NDI FALSE CACHE BOOL "Enable support for NDI")
set(TLRENDER_NDI_SDK "" CACHE PATH "Full path to the NDI SDK")
set(TLRENDER_NDI_DYNAMIC_LIBRARY CACHE FALSE BOOL "Use a dynamic loading of NDI library")
set(TLRENDER_BMD FALSE CACHE BOOL "Enable support for Blackmagic Design devices")
set(TLRENDER_BMD_SDK "" CACHE PATH "Full path to the Blackmagic Design SDK")
set(TLRENDER_GL TRUE CACHE BOOL "Enable OpenGL support")
set(TLRENDER_VK FALSE CACHE BOOL "Enable Vulkan support")
set(TLRENDER_TESTS TRUE CACHE BOOL "Build tests")
if(APPLE)
    set(TLRENDER_IGNORE_PREFIX_PATH_DEFAULT /opt/homebrew)
endif()
set(TLRENDER_IGNORE_PREFIX_PATH ${TLRENDER_IGNORE_PREFIX_PATH_DEFAULT} CACHE STRING "Ignore the given prefix path")
set(TLRENDER_GCOV FALSE CACHE BOOL "Enable gcov code coverage")
set(TLRENDER_GPROF FALSE CACHE BOOL "Enable gprof code profiling")

#-------------------------------------------------------------------------------
# Internal configuration

list(PREPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

if(TLRENDER_PYTHON)
    add_definitions(-DTLRENDER_PYTHON)
endif()
if(TLRENDER_USD)
    add_definitions(-DTLRENDER_USD)
endif()
if(TLRENDER_BMD)
    add_definitions(-DTLRENDER_BMD)
endif()
if(MRV2_BACKEND STREQUAL "VK")
    add_definitions(-DVULKAN_BACKEND)
else()
    add_definitions(-DOPENGL_BACKEND)
endif()
if(TLRENDER_IGNORE_PREFIX_PATH)
    set(CMAKE_IGNORE_PREFIX_PATH ${TLRENDER_IGNORE_PREFIX_PATH})
endif()

if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    add_definitions(-DTLRENDER_ASSERT)
endif()

if(TLRENDER_TESTS)
    add_definitions(-DTLRENDER_SAMPLE_DATA="${CMAKE_CURRENT_SOURCE_DIR}/etc/SampleData")
    set(CTEST_OUTPUT_ON_FAILURE ON)
    enable_testing()
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/deps
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/etc
    ${PROJECT_SOURCE_DIR}/tests)
if ("${TLRENDER_API}" STREQUAL "GL_4_1")
    include_directories(${PROJECT_SOURCE_DIR}/deps/glad_GL_4_1/include)
elseif ("${TLRENDER_API}" STREQUAL "GL_4_1_Debug")
    include_directories(${PROJECT_SOURCE_DIR}/deps/glad_GL_4_1_Debug/include)
elseif ("${TLRENDER_API}" STREQUAL "GLES_2")
    include_directories(${PROJECT_SOURCE_DIR}/deps/glad_GLES_2/include)
endif()
include_directories(${CMAKE_INSTALL_PREFIX}/include)  # for FLTK

if(TLRENDER_GPROF)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
endif()
if(TLRENDER_GCOV)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")# -fkeep-inline-functions")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")# -fkeep-inline-functions")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

set(CMAKE_FIND_FRAMEWORK LAST)

if(WIN32)
elseif(APPLE)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
else()
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "../lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#-------------------------------------------------------------------------------
# Dependencies

# Required dependencies
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Imath REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(ZLIB REQUIRED)
find_package(minizip REQUIRED)
find_package(OpenTimelineIO REQUIRED)
find_package(Freetype REQUIRED)

# GLFW dependency
if(TLRENDER_GLFW)
    find_package(glfw3 REQUIRED)
    add_definitions(-DTLRENDER_GLFW)
endif()

# OpenColorIO dependencies
if(TLRENDER_OCIO AND NOT "${TLRENDER_API}" STREQUAL "GLES_2")
    find_package(OpenColorIO REQUIRED)
    add_definitions(-DTLRENDER_OCIO)
endif()

# Audio dependencies
if(TLRENDER_AUDIO)
    find_package(RtAudio REQUIRED)
    add_definitions(-DTLRENDER_AUDIO)
endif()

# I/O dependencies
if(TLRENDER_JPEG)
    find_package(libjpeg-turbo)
    if(libjpeg-turbo_FOUND)
        add_definitions(-DTLRENDER_JPEG)
    endif()
endif()
if(TLRENDER_LIBPLACEBO)
    find_package(libplacebo)
    if(libplacebo_FOUND)
        add_definitions(-DTLRENDER_LIBPLACEBO)
    endif()
endif()
if(TLRENDER_TIFF)
    find_package(TIFF)
    if(TIFF_FOUND)
        add_definitions(-DTLRENDER_TIFF)
    endif()
endif()
if(TLRENDER_STB)
    add_definitions(-DTLRENDER_STB)
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
if(TLRENDER_NDI)
    find_package(NDI REQUIRED)
    if(NDI_FOUND)
        add_definitions(-DTLRENDER_NDI)
    endif()
endif()
if(TLRENDER_USD)
    find_package(pxr)
    if(pxr_FOUND)
        add_definitions(-DTLRENDER_USD)
    endif()
endif()
if(TLRENDER_RAW)
    find_package(LibRaw)
    if(LibRaw_FOUND)
        add_definitions(-DTLRENDER_RAW)
    endif()
endif()

# Python dependencies
if(TLRENDER_PYTHON)
    find_package(Python3 REQUIRED COMPONENTS Development)
endif()

#-------------------------------------------------------------------------------
# Sub-directories

add_definitions(-DTLRENDER_API_${TLRENDER_API})
if("${TLRENDER_API}" STREQUAL "GL_4_1_Debug")
    add_definitions(-DTLRENDER_API_GL_4_1)
endif()
if (TLRENDER_NDI_SDK MATCHES ".*Advanced.*")
    add_definitions(-DTLRENDER_NDI_ADVANCED)
endif()
add_subdirectory(deps/glad_${TLRENDER_API})

add_subdirectory(lib)
if(TLRENDER_TESTS)
    add_subdirectory(tests)
endif()
add_subdirectory(etc)
