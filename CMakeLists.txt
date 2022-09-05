cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)

project( mrv2 )
#SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".so")

include( cmake/options.cmake )
include( cmake/functions.cmake )
include( cmake/find_packages.cmake )

set( include_dirs
  ${FLTK_INCLUDE_DIRS}
  ${CMAKE_PREFIX_PATH}/include
  ${CMAKE_PREFIX_PATH}/include/OpenEXR
  ${CMAKE_PREFIX_PATH}/include/Imath
  ${CMAKE_PREFIX_PATH}/include/opentimelineio/deps
  ${CMAKE_CURRENT_BINARY_DIR}/mrvFl
  tlRender/lib
  tlRender/deps
  .
  )


include_directories(
  ${include_dirs}
  )

set( lib_dirs
  ${CMAKE_PREFIX_PATH}/lib
  tlRender/build/tlRender/src/tlRender-build/deps/tlGlad/
  )

link_directories( ${lib_dirs} )

list(PREPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules ${PROJECT_SOURCE_DIR}/tlRender/cmake/Modules)

add_definitions( -DUSE_GETTEXT )

# include(ExternalProject)

# set(TLRENDER_EXTERNAL_ARGS
#     -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
#     -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
#     -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
#     -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
#     -DCMAKE_POLICY_DEFAULT_CMP0091:STRING=NEW
#     -DCMAKE_POLICY_DEFAULT_CMP0114:STRING=NEW
#     -DCMAKE_FIND_FRAMEWORK=${CMAKE_FIND_FRAMEWORK}
#     -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
#     -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
#     -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
#     -DCMAKE_MSVC_RUNTIME_LIBRARY=${CMAKE_MSVC_RUNTIME_LIBRARY}
#     -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
#     -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
#     -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS})

# set(tlRender_ARGS
#     ${TLRENDER_EXTERNAL_ARGS}
#     -DTLRENDER_ENABLE_MMAP=${TLRENDER_ENABLE_MMAP}
#     -DTLRENDER_ENABLE_COVERAGE=${TLRENDER_ENABLE_COVERAGE}
#     -DTLRENDER_ENABLE_PYTHON=${TLRENDER_ENABLE_PYTHON}
#     -DTLRENDER_BUILD_GL=${TLRENDER_BUILD_GL}
#     -DTLRENDER_BUILD_BMD=${TLRENDER_BUILD_BMD}
#     -DTLRENDER_BMD_SDK=${TLRENDER_BMD_SDK}
#     -DTLRENDER_BUILD_QT5=${TLRENDER_BUILD_QT5}
#     -DTLRENDER_BUILD_QT6=${TLRENDER_BUILD_QT6}
#     -DTLRENDER_BUILD_PROGRAMS=${TLRENDER_BUILD_PROGRAMS}
#     -DTLRENDER_BUILD_EXAMPLES=${TLRENDER_BUILD_EXAMPLES}
#     -DTLRENDER_BUILD_TESTS=${TLRENDER_BUILD_TESTS})

# ExternalProject_Add(
#     tlRender-proj
#     PREFIX ${CMAKE_CURRENT_BINARY_DIR}/tlRender-proj
#     SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tlRender/etc/SuperBuild
#     INSTALL_COMMAND ""
#     CMAKE_ARGS ${tlRender_ARGS}
#     )

add_subdirectory( tlRender/etc/SuperBuild )
add_subdirectory( mrvCore )
add_subdirectory( mrvFl )
add_subdirectory( mrvFlGL )
# add_subdirectory( mrvFlApp )

set(HEADERS
  App.h
  Util.h)
set(SOURCES
  App.cpp
  Util.cpp
  main.cpp)

set(LIBRARIES tlApp mrvFlGL mrvFl )

add_executable(mrv2 ${SOURCES} ${HEADERS})
target_link_libraries(mrv2 PUBLIC ${LIBRARIES} )

install(TARGETS mrv2
  RUNTIME DESTINATION bin)
