cmake_minimum_required(VERSION 3.25)

project(mrv2)

set(CMAKE_CXX_STANDARD 20)

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
    
    # ---------------------------------------------------------------------------
    # NDI SDK architecture detection (Linux only)
    # For embedded/cross-compiled targets (HiSilicon, Rockchip, Sigma Star, etc.)
    # set NDI_SDK_ARCH manually in your toolchain file or on the cmake command line.
    # Desktop architectures are auto-detected from CMAKE_SYSTEM_PROCESSOR.
    # ---------------------------------------------------------------------------
    if(UNIX AND NOT APPLE AND NOT DEFINED NDI_SDK_ARCH)
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
            set(NDI_SDK_ARCH "x86_64-linux-gnu")

	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
            # Distinguish Raspberry Pi 4 (64-bit) from generic aarch64 desktop
            # (e.g. aarch64-newtek-linux-gnu covers Jetson, Apple M1 Linux, etc.)
            if(EXISTS "/proc/device-tree/model")
		file(READ "/proc/device-tree/model" _NDI_BOARD_MODEL)
		if(_NDI_BOARD_MODEL MATCHES "Raspberry Pi 4")
                    set(NDI_SDK_ARCH "aarch64-rpi4-linux-gnueabi")
		elseif(_NDI_BOARD_MODEL MATCHES "Rockchip")
                    set(NDI_SDK_ARCH "aarch64-rockchip-linux-gnu")
		else()
                    set(NDI_SDK_ARCH "aarch64-newtek-linux-gnu")
		endif()
		unset(_NDI_BOARD_MODEL)
            else()
		set(NDI_SDK_ARCH "aarch64-newtek-linux-gnu")
            endif()

	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(armv7l|armv7|armhf|arm)$")
            # Distinguish common 32-bit ARM desktop/SBC variants
            if(EXISTS "/proc/device-tree/model")
		file(READ "/proc/device-tree/model" _NDI_BOARD_MODEL)
		if(_NDI_BOARD_MODEL MATCHES "Raspberry Pi 4")
                    set(NDI_SDK_ARCH "arm-rpi4-linux-gnueabihf")
		elseif(_NDI_BOARD_MODEL MATCHES "Raspberry Pi 3")
                    set(NDI_SDK_ARCH "arm-rpi3-linux-gnueabihf")
		elseif(_NDI_BOARD_MODEL MATCHES "Raspberry Pi 2")
                    set(NDI_SDK_ARCH "arm-rpi2-linux-gnueabihf")
		elseif(_NDI_BOARD_MODEL MATCHES "Raspberry Pi")
                    set(NDI_SDK_ARCH "arm-rpi1-linux-gnueabihf")
		elseif(_NDI_BOARD_MODEL MATCHES "Rockchip")
                    set(NDI_SDK_ARCH "arm-rockchip-linux-gnueabihf")
		else()
                    set(NDI_SDK_ARCH "arm-newtek-linux-gnueabihf")
		endif()
		unset(_NDI_BOARD_MODEL)
            else()
		set(NDI_SDK_ARCH "arm-newtek-linux-gnueabihf")
            endif()

	else()
            message(WARNING
		"NDI: Unrecognised architecture '${CMAKE_SYSTEM_PROCESSOR}'. "
		"Falling back to x86_64-linux-gnu. "
		"For embedded targets (HiSilicon himix/hisiv, Sigma Star, etc.) "
		"set -DNDI_SDK_ARCH=<dir> on the cmake command line.")
            set(NDI_SDK_ARCH "x86_64-linux-gnu")
	endif()

	message(STATUS "NDI SDK arch directory: ${NDI_SDK_ARCH}")
    endif()
endif()

include(cmake/version.cmake )
include(cmake/options.cmake )
include(cmake/functions.cmake )

# Use ccache if available to speed up rebuilds
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Found ccache: ${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})

    if(MSVC)
	# Replace /Zi or /ZI with /Z7 for ccache compatibility
	string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG
	    "${CMAKE_CXX_FLAGS_DEBUG}")
	string(REPLACE "/ZI" "/Z7" CMAKE_CXX_FLAGS_DEBUG
	    "${CMAKE_CXX_FLAGS_DEBUG}")
	string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_DEBUG
	    "${CMAKE_C_FLAGS_DEBUG}")
    endif()
endif()

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

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" SYSTEM_PROCESSOR_LC)

set(vcpkg_ARCH x64)
if(SYSTEM_PROCESSOR_LC MATCHES "^(aarch64|arm64)$")
    set(vcpkg_ARCH arm64)
elseif(SYSTEM_PROCESSOR_LC MATCHES ".*(amd64|x86_64).*")
    set(SYSTEM_PROCESSOR_LC amd64)
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
#
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


# Do not use Windows' registry to find Python
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

#
# Python Build / Finding
#
set(Python_FIND_DEBUG TRUE)
set(Python3_FIND_DEBUG TRUE)
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

set(SHADERC_DEP )


add_subdirectory(tlRender/etc/SuperBuild)

set(FLTK_DEP )
if(BUILD_FLTK)
    include(cmake/Modules/BuildFLTK.cmake)
    add_dependencies(tlRender ${FLTK_DEP})
endif()

# OpenSSL must be built before Python and pyFLTK to use the right path when
# building Python's _ssl module.
if(BUILD_PYTHON)
    if(TLRENDER_NET)
	if (OpenSSL_DEP)
	    add_dependencies(Python ${OpenSSL_DEP})
	else()
	    message(FATAL_ERROR "Unknown architecture for Python and OpenSSL")
	endif()
    endif()
endif()

set(pyFLTK_DEP )
if(MRV2_PYFLTK)
    include(cmake/Modules/BuildpyFLTK.cmake)
endif()



set(POCO_DEP )
if(MRV2_NETWORK)
    include(cmake/Modules/BuildPoco.cmake)
    include(cmake/Modules/Buildlibdatachannel.cmake)
endif()

if(BUILD_MRV2)
    include(cmake/Modules/Buildmrv2.cmake)
endif()

