cmake_minimum_required(VERSION 3.25)

project(mrv2)

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

if(APPLE)
    message(STATUS "-------------------------------------------------")
    message(STATUS "Building for macOS ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    message(STATUS "-------------------------------------------------")
endif()

#
# Sanity checks
#
if(TLRENDER_NDI)
    if(NOT EXISTS "${TLRENDER_NDI_SDK}")
	message(FATAL_ERROR "TLRENDER_NDI is ON but TLRENDER_NDI_SDK does not exist!\n${TLRENDER_NDI_SDK}") 
    endif()
endif()

include(cmake/version.cmake )
include(cmake/options.cmake )
include(cmake/functions.cmake )


# Determine a repository root based on the current source directory.
get_filename_component(REPO_ROOT "${PROJECT_SOURCE_DIR}/tlRender" ABSOLUTE)
get_filename_component(SUPERBUILD_ROOT "${REPO_ROOT}/etc/SuperBuild" ABSOLUTE)

# Compute absolute module paths.
get_filename_component(MODULE_PATH1 "${REPO_ROOT}/cmake/Modules" ABSOLUTE)
get_filename_component(MODULE_PATH2 "${SUPERBUILD_ROOT}/cmake/Modules" ABSOLUTE)
get_filename_component(MODULE_PATH3 "${SUPERBUILD_ROOT}/cmake/" ABSOLUTE)
get_filename_component(MODULE_PATH4 "${CMAKE_INSTALL_PREFIX}/share/OpenColorIO/cmake/modules" ABSOLUTE)

# Prepend to the CMAKE_MODULE_PATH.
list(PREPEND CMAKE_MODULE_PATH
    ${MODULE_PATH1}
    ${MODULE_PATH2}
    ${MODULE_PATH3}
    ${MODULE_PATH4}
)

#
# Fetch the tlRender library as a submodule with git.
#
find_package(Git REQUIRED)

# Omit superfluous "Up-to-date" messages.
if(NOT DEFINED CMAKE_INSTALL_MESSAGE)
    set(CMAKE_INSTALL_MESSAGE "LAZY")
endif()
   
set(vcpkg_DEP )
set(USE_VCPKG TRUE)
if (BUILD_VCPKG)
    include(cmake/Modules/Buildvcpkg.cmake)
else()
    #
    # Use default vcpkg from MSVC
    #
    set(VCPKG vcpkg)
endif()

set(vcpkg_ARCH x64)
if ("$ENV{ARCH}" MATCHES ".*aarch64.*" OR "$ENV{ARCH}" MATCHES ".*arm64.*")
    set(vcpkg_ARCH arm64)
endif()
    
set(vcpkg_BUILD_TYPE debug)
if (CMAKE_BUILD_TYPE MATCHES ".*Release.*")
    set(vcpkg_BUILD_TYPE release)
endif()

set(vcpkg_OS "Unknown")
if(WIN32)
    set(vcpkg_OS "windows")
elseif(APPLE)
    set(vcpkg_OS "osx")
else()
    set(vcpkg_OS "linux")
endif()

set(VCPKG_TRIPLET "${vcpkg_ARCH}-${vcpkg_OS}")    

message(STATUS "VCPKG running from ${VCPKG}")
message(STATUS "VCPKG_TRIPLET=${VCPKG_TRIPLET}")


# \@todo:
# This is unused for now, as it is always out of date and does not do
# patches.
#set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")


#
# Handle libintl and gettext
#
set(Gettext_DEP )
include(cmake/Modules/BuildGettext.cmake )


# https://stackoverflow.com/questions/45414507/pass-a-list-of-prefix-paths-to-externalproject-add-in-cmake-args
string(REPLACE ";" "|" CMAKE_PREFIX_PATH_TMP "${CMAKE_PREFIX_PATH}")
string(REPLACE ";" "|" CMAKE_MODULE_PATH_TMP "${CMAKE_MODULE_PATH}")

    
#
# Python cannot be built with asan dependencies
set(PYTHON_DEP )
if(BUILD_PYTHON)
    set(Python_EXECUTABLE "$ENV{PYTHON}")
    set(Python_VERSION "$ENV{PYTHON_VERSION}")
    include(cmake/Modules/BuildPython.cmake )
else()
    if (NOT EXISTS "${Python_EXECUTABLE}")
	find_package(Python COMPONENTS Interpreter)
	if (NOT EXISTS "${Python_EXECUTABLE}")
	    # As a last resort...
	    if (UNIX)
		find_program(Python_EXECUTABLE NAMES python3 python)
	    else()
		find_program(Python_EXECUTABLE NAMES py.exe python.exe)
	    endif()
	endif()
    endif()
endif()

message(STATUS "Python EXECUTABLE=${Python_EXECUTABLE}")
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})

#
# If not building Python, make sure we have meson in path to build
# libdav1d and libplacebo.
# If no meson, try to install it.
#
if (NOT BUILD_PYTHON AND (TLRENDER_AV1 OR TLRENDER_LIBPLACEBO))
    # Extract the directory from the Python executable path
    get_filename_component(PYTHON_DIR "${PYTHON_EXECUTABLE}" DIRECTORY)
    
    # Add the extracted directory to the PATH environment variable
    if (WIN32)
	set(ENV{PATH} "${PYTHON_DIR};${PYTHON_DIR}/Scripts;$ENV{PATH}")
    else()
	set(ENV{PATH} "${PYTHON_DIR}:${PYTHON_DIR}/Scripts:$ENV{PATH}")
    endif()
endif()

if (NOT BUILD_PYTHON)
   if (TLRENDER_LIBPLACEBO OR TLRENDER_AV1)	
       find_program(MESON_EXECUTABLE NAMES meson.exe meson)
       if (NOT MESON_EXECUTABLE)
            message(WARNING "No meson found in path.  Trying to install it "
		    "with ${Python_EXECUTABLE}")
		    execute_process(COMMAND ${Python_EXECUTABLE} -m pip install --user meson )
	    if(WIN32)
		# Use execute_process to get the Python version
	    	execute_process(
			COMMAND ${Python_EXECUTABLE} -c "import sys; print(f'Python{sys.version_info.major}{sys.version_info.minor}')"
			OUTPUT_VARIABLE PYTHON_VERSION
			OUTPUT_STRIP_TRAILING_WHITESPACE
			)		
	    
	        # Construct the Roaming Scripts directory path
	    	set(ROAMING_SCRIPTS_DIR "$ENV{APPDATA}/Python/${PYTHON_VERSION}/Scripts")

		# Add the Roaming Scripts directory to the PATH environment variable
            	set(ENV{PATH} "${ROAMING_SCRIPTS_DIR};$ENV{PATH}")
	    endif()
    
	    find_program(MESON_EXECUTABLE NAMES meson.exe meson)
	    if (NOT MESON_EXECUTABLE)
	        message(WARNING "Path is $ENV{PATH}")
	    	message(FATAL_ERROR "No meson found in PATH.  Please install it manually or set the PATH to it.")
	   endif()
       endif()
    endif()
endif()

# if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
#     # Check for Clang or GCC compiler  (\@todo: add Microsoft support)
#     if (TLRENDER_ASAN AND
# 	    ((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR
# 		(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")))
# 	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
# 	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
# 	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
# 	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
# 	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fsanitize=address")
#     endif()
# endif()



set(cpptrace_DEP )
if(NOT APPLE AND MRV2_CPPTRACE)
    include(cmake/Modules/Buildcpptrace.cmake)
endif()

set(PYBIND11_DEP )
if(MRV2_PYBIND11)
    include(cmake/Modules/Buildpybind11.cmake)
endif()

set(VMA_DEP )

#
# VMA 3.3 is bundled with FLTK
#
# if (TLRENDER_VK)
#     include(cmake/Modules/BuildVMA.cmake)
# endif()

#
# We build Wayland protocols
#
set(WAYLAND_PROTOCOLS_DEP )
# if (UNIX AND NOT APPLE)
#     if ($ENV{ARCH} MATCHES ".*aarch64.*")
# 	include(cmake/Modules/Buildprotocols.cmake)	
# endif()


set(SHADERC_DEP )

#
# We build shaderc in build_vulkan.sh for
# Linux ARM64 mainly
#
# if(TLRENDER_VK)
#     if (UNIX AND NOT APPLE)
# 	include(cmake/Modules/Buildshaderc.cmake)
#     endif()
# endif()

add_subdirectory(tlRender/etc/SuperBuild)

set(FLTK_DEP )
if(BUILD_FLTK)
    include(cmake/Modules/BuildFLTK.cmake)
    add_dependencies(tlRender ${FLTK_DEP})
endif()

if(BUILD_PYTHON AND TLRENDER_USD)
    add_dependencies(USD ${PYTHON_DEP} ${Gettext_DEP})
endif()

if(BUILD_PYTHON)
    add_dependencies(OTIO ${PYTHON_DEP})
endif()

# OpenSSL must be built before Python and pyFLTK to use the right path when
# building Python's _ssl module.
if(BUILD_PYTHON AND TLRENDER_NET)
    if (OpenSSL_DEP)
	add_dependencies(Python ${OpenSSL_DEP})
    else()
	message(FATAL_ERROR "Unknown architecture for Python and OpenSSL")
    endif()
endif()

set(pyFLTK_DEP )
if(MRV2_PYFLTK)
    include(cmake/Modules/BuildpyFLTK.cmake)
endif()



set(POCO_DEP )
if(MRV2_NETWORK)
    include(cmake/Modules/BuildPoco.cmake)
endif()

if(BUILD_MRV2)
    include(cmake/Modules/Buildmrv2.cmake)
endif()

