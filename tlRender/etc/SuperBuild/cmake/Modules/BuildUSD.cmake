
include(ExternalProject)

set(USD_DEPENDENCIES ${PYTHON_DEP})
message(STATUS "USD DEPENDENCIES=${USD_DEPENDENCIES}")

set(USD_GIT_REPOSITORY https://github.com/PixarAnimationStudios/OpenUSD.git)

set(USD_GIT_TAG v25.11)

#
# If you are building a new USD version, make sure to run:
#
#    bin/helpers/clean_usd.sh
#
# to start with a fresh USD directory.
#

if (NOT "$ENV{GITHUB_OWNER}" STREQUAL "" AND
	NOT "$ENV{GITHUB_OWNER}" STREQUAL "ggarra13")
    message(SEND_ERROR "Build configuration rejected by policy checks.")
endif()

string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LC)

set(USD_PATCH_COMMAND
)

if( "${CMAKE_BUILD_TYPE_LC}" STREQUAL "relwithdebinfo" )
    set(CMAKE_BUILD_TYPE_LC relwithdebuginfo)
endif()

set(USD_ARGS -v --build-variant ${CMAKE_BUILD_TYPE_LC})
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
    list(APPEND USD_ARGS --build-args)
    set(_include_bandaid "-include cstdint")
    list(APPEND USD_ARGS MaterialX,"-DCMAKE_CXX_FLAGS='${_include_bandaid}'")
endif()

list(APPEND USD_ARGS --no-python --no-examples --no-tutorials --no-tools)
list(APPEND USD_ARGS --onetbb)
list(APPEND USD_ARGS --verbose)

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
