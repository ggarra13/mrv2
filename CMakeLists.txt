cmake_minimum_required(VERSION 3.25)

project(mrv2)

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)
if( POLICY CMP0135 )
    cmake_policy(SET CMP0135 NEW)
endif()

if(APPLE)
    message( STATUS "-------------------------------------------------" )
    message( STATUS "Building for macOS ${CMAKE_OSX_DEPLOYMENT_TARGET}" )
    message( STATUS "-------------------------------------------------" )
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

#
# Python cannot be built with asan dependencies
#set(PYTHON_DEP )
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

if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    # Check for Clang or GCC compiler  (\@todo: add Microsoft support)
    if (TLRENDER_ASAN AND
	    ((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR
		(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")))
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fsanitize=address")
    endif()
endif()


set( Gettext_DEP )
if(BUILD_GETTEXT)
    include(cmake/Modules/BuildGettext.cmake )
elseif(APPLE)
    if (NOT CMAKE_OSX_ARCHITECTURES OR CMAKE_OSX_ARCHITECTURES STREQUAL "")
	message(WARNING
	    "Target architecture not specified for macOS deployment")
    else()
	if(CMAKE_OSX_ARCHITECTURES MATCHES "^x86_64$") 
	    #
	    # brew .dylibs are compiled only for the current version of the OS.
	    #
	    # We distribute libintl dylib compatible with v10.5 on Intel macOS
	    file(COPY
		precompiled/macOS/v10.5/ DESTINATION ${CMAKE_INSTALL_PREFIX}/
		FOLLOW_SYMLINK_CHAIN)
	endif()
    endif()
endif()


set(cpptrace_DEP )
if(NOT APPLE)
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

add_subdirectory( tlRender/etc/SuperBuild)

set(FLTK_DEP )
if(BUILD_FLTK)
    include(cmake/Modules/BuildFLTK.cmake)
    add_dependencies(tlRender ${FLTK_DEP})
endif()

if(BUILD_PYTHON AND TLRENDER_USD)
    add_dependencies(USD ${PYTHON_DEP})
endif()

set(pyFLTK_DEP )
if(MRV2_PYFLTK)
    include(cmake/Modules/BuildpyFLTK.cmake)
endif()

# OpenSSL must be built before Python and pyFLTK to use the right path when
# building Python's _ssl module.
if(TLRENDER_NET AND NOT WIN32)
    if (BUILD_PYTHON)
	add_dependencies(Python OpenSSL)
    endif()
endif()


set(POCO_DEP )
if(MRV2_NETWORK)
    include(cmake/Modules/BuildPoco.cmake)
endif()

if(BUILD_MRV2)
    include(cmake/Modules/Buildmrv2.cmake)
endif()

