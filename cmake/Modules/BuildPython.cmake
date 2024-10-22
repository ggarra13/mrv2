# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )


include(ProcessorCount)
ProcessorCount(NPROCS)


set( Python_VERSION 3.11 )
set( Python_TINY    9 )

#set( Python_VERSION 3.12 )
#set( Python_TINY   3 )

set( Python_URL https://www.python.org/ftp/python/${Python_VERSION}.${Python_TINY}/Python-${Python_VERSION}.${Python_TINY}.tar.xz )

if(UNIX)
    set(Python_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/python${Python_VERSION})
else()
    set(Python_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/python.exe )
endif()

    
set( Python_PATCH )
set( Python_ENV )
set( Python_PATH $ENV{PATH} )
if(APPLE)

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
    
    set( Python_CONFIGURE ${CMAKE_COMMAND} -E env "CFLAGS=${Python_C_FLAGS}" "CPPFLAGS=${Python_C_FLAGS}" "CXXFLAGS=${Python_CXX_FLAGS}" "LDFLAGS=${Python_LD_FLAGS}" -- ./configure --enable-optimizations --enable-shared --with-openssl=${_openssl_LOC} --prefix=${CMAKE_INSTALL_PREFIX}
    )
    set( Python_BUILD  make -j ${NPROCS} )
    set( Python_INSTALL
	COMMAND make -j ${NPROCS} install
	COMMAND ${Python_EXECUTABLE} -m ensurepip --upgrade )
    set( Python_INSTALL  make -j ${NPROCS} install )

elseif(UNIX)

    set(Python_C_FLAGS "${CMAKE_C_FLAGS}" )
    set(Python_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
    set(Python_LD_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" )

    set( Python_CONFIGURE ${CMAKE_COMMAND} -E env "CFLAGS=${Python_C_FLAGS}" "CPPFLAGS=${Python_C_FLAGS}" "CXXFLAGS=${Python_CXX_FLAGS}" "LDFLAGS=${Python_LD_FLAGS}" -- ./configure
	--enable-optimizations
	--enable-shared
        --prefix=${CMAKE_INSTALL_PREFIX}
    )
    set( Python_BUILD   make -j ${NPROCS} )
    set( Python_INSTALL
	COMMAND make -j ${NPROCS} install
	COMMAND ${Python_EXECUTABLE} -m ensurepip --upgrade )
else()

    set( platform x64 )
    if( "$ENV{ARCH}" STREQUAL "i386" )
    	set( platform Win32 )
    endif()

    # @bug:
    #    This creates another .bat file and makes the original .bat file fail
    # set( Python_ENV ${CMAKE_COMMAND} -E env "PATH=${Python_PATH}" --)
    
    list(JOIN Python_PATH "|" Python_PATH_STR)
    set(Python_SCRIPT "-DPATH=${Python_PATH_STR}" -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -P ${PROJECT_SOURCE_DIR}/cmake/build_python_windows.cmake
    )

    set(Python_PATCH
	COMMAND
	${CMAKE_COMMAND} -E copy_if_different
	"${PROJECT_SOURCE_DIR}/cmake/patches/Python-patch/PC/layout/main.py"
	"${CMAKE_BINARY_DIR}/deps/Python/src/Python/PC/layout"
    )


    set(Python_BUILD ${CMAKE_COMMAND} -D Python_COMMAND=build
	-D Python_PLATFORM=${platform})
    set(Python_BUILD ${Python_BUILD} ${Python_SCRIPT})
    
    set(Python_INSTALL
	COMMAND ${CMAKE_COMMAND} -D Python_COMMAND=install ${Python_SCRIPT}
	COMMAND ${CMAKE_COMMAND} -D Python_COMMAND=pip ${Python_SCRIPT})
endif()

ExternalProject_Add(
    Python
    URL ${Python_URL}
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/deps/Python
    PATCH_COMMAND     "${Python_PATCH}"
    CONFIGURE_COMMAND "${Python_CONFIGURE}"
    BUILD_COMMAND     "${Python_BUILD}"
    INSTALL_COMMAND   "${Python_INSTALL}"
    BUILD_IN_SOURCE 1
)


set( PYTHON_DEP Python )

