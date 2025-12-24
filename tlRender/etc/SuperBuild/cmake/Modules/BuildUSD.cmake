
include(ExternalProject)

set(USD_DEPENDENCIES ${PYTHON_DEP})
message(STATUS "USD DEPENDENCIES=${USD_DEPENDENCIES}")

set(USD_GIT_REPOSITORY https://github.com/PixarAnimationStudios/OpenUSD.git)

include(functions/detect_new_linux_distro)

set(IS_NEW_LINUX FALSE)
if (UNIX AND NOT APPLE)
    detect_new_linux_distro(IS_NEW_LINUX)
endif()

# v25.11 latest (stops on TBB compilation with and without --onetbb)
if (IS_NEW_LINUX)
    set(USD_GIT_TAG v25.02a) # v25.02a works
else()
    set(USD_GIT_TAG v25.11) # v25.02a works
endif()

message(STATUS "Is new Linux=${IS_NEW_LINUX} Compiling USD: ${USD_GIT_TAG}")

string(TOLOWER ${CMAKE_BUILD_TYPE} cmake_build_type)

set(USD_PATCH_COMMAND
)

if( "${cmake_build_type}" STREQUAL "relwithdebinfo" )
    set(cmake_build_type relwithdebuginfo)
endif()

set(USD_ARGS --build-variant ${cmake_build_type})
if(APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
    list(APPEND USD_ARGS --build-args)
    list(APPEND USD_ARGS USD,"-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    list(APPEND USD_ARGS OpenSubdiv,"-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    list(APPEND USD_ARGS MaterialX,"-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    list(APPEND USD_ARGS
        oneTBB,-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
        oneTBB,-DCMAKE_CXX_OSX_DEPLOYMENT_TARGET_FLAG:STRING="-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}"
        oneTBB,-DCMAKE_C_OSX_DEPLOYMENT_TARGET_FLAG:STRING="-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
elseif(UNIX AND NOT APPLE)
    #
    # Pixar's repository distributes code without <cstdint> which will
    # choke on modern compilers.  This is a bandaid fix to include
    # <cstdint> on all files compiled.
    #
    list(APPEND USD_ARGS --build-args)
    set(_include_bandaid "-include cstdint")
    list(APPEND USD_ARGS USD,"-DCMAKE_CXX_FLAGS='${_include_bandaid}'")
    list(APPEND USD_ARGS OpenSubdiv,"-DCMAKE_CXX_FLAGS='${_include_bandaid}'")
    list(APPEND USD_ARGS MaterialX,"-DCMAKE_CXX_FLAGS='${_include_bandaid}'")
endif()

list(APPEND USD_ARGS --no-python --no-examples --no-tutorials --no-tools)
if (NOT IS_NEW_LINUX)
    list(APPEND USD_ARGS --onetbb)
endif()
list(APPEND USD_ARGS --generator Ninja -v)  # repeat -v up to three times for more verbose

set(USD_INSTALL_COMMAND )
if(WIN32)
    # \todo On Windows the USD cmake build system installs the "*.dll" files
    # and "usd" directory into "lib", however it seems like they need to be
    # in "bin" instead.
    cmake_path(CONVERT ${CMAKE_INSTALL_PREFIX} TO_NATIVE_PATH_LIST cmake_install)
    set(USD_INSTALL_COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${CMAKE_INSTALL_PREFIX}/lib/usd  ${CMAKE_INSTALL_PREFIX}/bin/usd
        COMMAND copy "${cmake_install}\\lib\\*.dll" "${cmake_install}\\bin")
endif()

ExternalProject_Add(
    USD
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/USD
    DEPENDS ${USD_DEPENDENCIES}
    GIT_REPOSITORY ${USD_GIT_REPOSITORY}
    GIT_TAG ${USD_GIT_TAG}
    
    CONFIGURE_COMMAND ""
    PATCH_COMMAND ${USD_PATCH_COMMAND}
    BUILD_COMMAND ${CMAKE_COMMAND} -E env "DYLD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib" -- ${TLRENDER_USD_PYTHON} build_scripts/build_usd.py ${CMAKE_INSTALL_PREFIX} ${USD_ARGS} 
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND "${USD_INSTALL_COMMAND}")
