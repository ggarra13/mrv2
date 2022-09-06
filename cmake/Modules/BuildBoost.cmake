#---------------------------------------------------------------------------
# Get and build boost

include( ExternalProject )

set( Boost_Bootstrap_Command )
if( UNIX )
  set( Boost_Bootstrap_Command ./bootstrap.sh )
  set( Boost_b2_Command b2 )
else()
  if( WIN32 )
    set( Boost_Bootstrap_Command bootstrap.bat )
    set( Boost_b2_Command b2.exe )
  endif()
endif()

set( BOOST_URL "https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/boost_1_73_0.tar.bz2" )

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
    --Variant=release
    -j8
  INSTALL_COMMAND ""
  INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
)

set( BOOST BOOST )
