# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )


include(ProcessorCount)
ProcessorCount(NPROCS)

set( Python_REPOSITORY https://github.com/python/cpython)

set( Python_VERSION 3.12 )  # was 3.11.9
set( Python_TINY    12 )

set( Python_GIT_TAG v${Python_VERSION}.${Python_TINY})

set( Python_DEPENDENCIES ${OpenSSL_DEP} )

if(UNIX)
    set(Python_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/python${Python_VERSION})
else()
    set(Python_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/python.exe )
endif()

    
set( Python_PATCH )
set( Python_ENV )
set( Python_PATH $ENV{PATH} )

if(APPLE)

    set(Python_DYLD_LIBRARY_PATH $ENV{DYLD_LIBRARY_PATH})
    set(Python_C_FLAGS "${CMAKE_C_FLAGS}" )
    set(Python_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
    set(Python_LD_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" )
    
    if(CMAKE_OSX_DEPLOYMENT_TARGET)
	set( Python_C_FLAGS "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} ${CMAKE_C_FLAGS}")
    endif()

    if(TLRENDER_NET)
	set(_openssl_LOC ${CMAKE_PREFIX_PATH})
    else()
	execute_process(
	    COMMAND brew --prefix openssl
	    OUTPUT_VARIABLE _openssl_LOC
	    OUTPUT_STRIP_TRAILING_WHITESPACE
	    ERROR_VARIABLE openssl_prefix_error
	)
	
	if(openssl_prefix_error)
	    message(FATAL_ERROR "Could not located openssl with 'brew --prefix openssl'.  Error: ${openssl_prefix_error}")
	endif()
	
    endif()
    
    set(Python_PATCH
	COMMAND
	${CMAKE_COMMAND} -E copy_if_different
	"${PROJECT_SOURCE_DIR}/cmake/patches/Python-patch/configure"
	"${CMAKE_BINARY_DIR}/deps/Python/src/Python/"
	COMMAND chmod 0755 "${CMAKE_BINARY_DIR}/deps/Python/src/Python/configure"
    )
    
    set( Python_ENV ${CMAKE_COMMAND} -E env "DYLD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib:${Python_DYLD_LIBRARY_PATH}" -- )
    set( Python_CONFIGURE ${CMAKE_COMMAND} -E env "CFLAGS=${Python_C_FLAGS}" "CPPFLAGS=${Python_C_FLAGS}" "CXXFLAGS=${Python_CXX_FLAGS}" "LDFLAGS=${Python_LD_FLAGS}" "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}" -- ./configure --enable-optimizations --enable-shared --with-openssl=${_openssl_LOC} --prefix=${CMAKE_INSTALL_PREFIX}
    )
    set( Python_BUILD make -j ${NPROCS} )
    set( Python_INSTALL
	COMMAND make -j ${NPROCS} install
	COMMAND ${Python_ENV} ${Python_EXECUTABLE} -m ensurepip --upgrade)

elseif(UNIX)
    
    set(Python_LD_LIBRARY_PATH $ENV{LD_LIBRARY_PATH})
    set(Python_C_FLAGS "${CMAKE_C_FLAGS}" )
    set(Python_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
    set(Python_LD_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" )

    # The standard library is in 'Lib' in the source directory during the build
    set(Python_SOURCE_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/deps/Python/src/Python/Lib")
    
    # Existing ENV setting for shared library path
    set( Python_ENV ${CMAKE_COMMAND} -E env "LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib:${Python_LD_LIBRARY_PATH}" "PYTHONPATH=${Python_SOURCE_LIB_DIR}" "CFLAGS=${Python_C_FLAGS}" "CPPFLAGS=${Python_C_FLAGS}" "CXXFLAGS=${Python_CXX_FLAGS}" "LDFLAGS=${Python_LD_FLAGS}" -- )

    set( Python_CONFIGURE ${Python_ENV} ./configure
	--enable-optimizations
	--enable-shared
        --prefix=${CMAKE_INSTALL_PREFIX}
	--without-ensurepip
    )
    
    # Build command
    set( Python_BUILD
	COMMAND ${Python_ENV} make -j ${NPROCS}
    )

    # --- NEW: Environment for INSTALL/POST-INSTALL (Needed for 'ensurepip') ---
    # Once installed, the Python executable relies on PYTHONHOME to find its standard library
    set( Python_INSTALL_ENV ${CMAKE_COMMAND} -E env
        "LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib:${Python_LD_LIBRARY_PATH}"
        "PYTHONHOME=${CMAKE_INSTALL_PREFIX}" # Crucial for finding 'encodings' after installation
        --
    )
    
    # Existing INSTALL commands (Use Python_ENV as defined above)
    set( Python_INSTALL
        COMMAND ${Python_ENV} make -j ${NPROCS} install
        COMMAND ${Python_INSTALL_ENV} ${Python_EXECUTABLE} -m ensurepip --upgrade 
        COMMAND ${Python_INSTALL_ENV} ${Python_EXECUTABLE} -m pip install meson
    )

else()
    
    set( platform x64 )
    if( "$ENV{ARCH}" MATCHES ".*aarch64.*" OR "$ENV{ARCH}" MATCHES ".*arm64.*" )
    	set( platform ARM64 )
    elseif( "$ENV{ARCH}" STREQUAL "i386" )
    	set( platform Win32 )
    endif()

    # @bug:
    #    This creates another .bat file and makes the original .bat file fail
    # set( Python_ENV ${CMAKE_COMMAND} -E env "PATH=${Python_PATH}" --)
    
    list(JOIN Python_PATH "|" Python_PATH_STR)
    set(Python_SCRIPT "-DPATH=${Python_PATH_STR}" -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -P ${PROJECT_SOURCE_DIR}/cmake/build_python_windows.cmake
    )


    set(Python_BUILD ${CMAKE_COMMAND} -D Python_COMMAND=build
	-D Python_PLATFORM=${platform})
    set(Python_BUILD ${Python_BUILD} ${Python_SCRIPT})
    
    set(Python_INSTALL
	COMMAND ${CMAKE_COMMAND} -D Python_COMMAND=install ${Python_SCRIPT}
	COMMAND ${CMAKE_COMMAND} -D Python_COMMAND=pip ${Python_SCRIPT}
	COMMAND ${CMAKE_COMMAND} -D Python_COMMAND=meson ${Python_SCRIPT})
endif()

ExternalProject_Add(
    Python
    GIT_REPOSITORY ${Python_REPOSITORY}
    GIT_TAG ${Python_GIT_TAG}
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/deps/Python

    DEPENDS ${Python_DPENDENCIES}
    
    PATCH_COMMAND     "${Python_PATCH}"
    CONFIGURE_COMMAND "${Python_CONFIGURE}"
    BUILD_COMMAND     "${Python_BUILD}"
    INSTALL_COMMAND   "${Python_INSTALL}"
    BUILD_IN_SOURCE 1
)

set(TLRENDER_USD_PYTHON ${Python_EXECUTABLE})

if (WIN32)
    set( ENV{PATH} "${CMAKE_INSTALL_PREFIX}/bin;${CMAKE_INSTALL_PREFIX}/bin/Scripts/;$ENV{PATH}" )
else()
    set( ENV{PATH} "${CMAKE_INSTALL_PREFIX}/bin:$ENV{PATH}" )
    if (UNIX AND NOT APPLE)
	set( ENV{PYTHONPATH} "${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION}:${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION}/site-packages:${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION}/lib-dynload:$ENV{PYTHONPATH}" )
    endif()
endif()

set( PYTHON_DEP Python )

