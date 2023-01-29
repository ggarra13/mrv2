# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

#---------------------------------------------------------------------------
# Get and build boost

include( ExternalProject )

set( BOOST_VERSION 1_80_0 )

set( Boost_Bootstrap_Command )
if( UNIX )
    set( Boost_Bootstrap_Command ./bootstrap.sh --with-libraries=filesystem,regex )
    set( Boost_b2_Command ./b2 )
else()
    if( WIN32 )
	set( Boost_Bootstrap_Command ./bootstrap.bat )
	set( Boost_b2_Command ./b2.exe )
    endif()
endif()

set( BOOST_VARIANT release )
if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
    set( BOOST_VARIANT debug )
endif()

set( BOOST_URL "https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_${BOOST_VERSION}.tar.bz2" )

set( BOOST_ARCHITECTURE )
if ( CMAKE_OSX_ARCHITECTURES MATCHES "arm64" )
    set( BOOST_ARCHITECTURE architecture=combinerd )
endif()



ExternalProject_Add(
    BOOST
    URL ${BOOST_URL}
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ${Boost_Bootstrap_Command}
    BUILD_COMMAND  ${Boost_b2_Command} install
    --without-container
    --without-context
    --without-contract
    --without-coroutine
    --without-exception
    --without-fiber
    --without-graph
    --without-graph_parallel
    --disable-icu
    --without-iostreams
    --without-json
    --without-log
    --without-program_options
    --without-math
    --without-mpi
    --without-nowide
    --without-python
    --without-random
    --without-stacktrace
    --without-test
    --without-timer
    --without-type_erasure
    --without-wave
    --prefix=${CMAKE_INSTALL_PREFIX}
    --threading=multi
    link=static
    runtime-link=shared
    variant=${BOOST_VARIANT}
    cxxflags=${CMAKE_CXX_FLAGS}
    ${BOOST_ARCHITECTURE}
    -j8
    INSTALL_COMMAND ""
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
)

set( BOOST BOOST )
