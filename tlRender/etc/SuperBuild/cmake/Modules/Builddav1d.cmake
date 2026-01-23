include(ExternalProject)

include(functions/find_meson_executable)
find_meson_executable(dav1d)


set(dav1d_GIT_TAG 1.3.0)

set(dav1d_DEPENDENCIES ${PYTHON_DEP})
if(UNIX)
    list(APPEND dav1d_DEPENDENCIES NASM ${dav1d_DEPS})
endif()

message(STATUS "dav1d DEPENDENCIES=${dav1d_DEPENDENCIES}")

set(CLANG_ENV )
if(WIN32)
    set(CLANG_ENV CC=clang CXX=clang)
endif()

if(APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
    set(dav1d_CFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set(dav1d_CXXFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set(dav1d_LDFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()


set(dav1d_RENAME_TO_LIB )
if (WIN32)
    set(dav1d_RENAME_TO_LIB
	COMMAND cmake -E rename ${CMAKE_INSTALL_PREFIX}/lib/libdav1d.a
	 ${CMAKE_INSTALL_PREFIX}/lib/dav1d.lib)
endif()

set(dav1d_CONFIGURE
    COMMAND ${CMAKE_COMMAND} -E env
    ${CLANG_ENV}
    "CXXFLAGS=${dav1d_CXXFLAGS}"
    "CFLAGS=${dav1d_CFLAGS}"
    "LDFLAGS=${dav1d_LDFLAGS}"
    "LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib"
    ${dav1d_PYTHONPATH} 
    -- ${MESON_EXECUTABLE} setup
    --wipe
    -Denable_tools=false
    -Denable_tests=false
    --default-library=static
    -Dlibdir=${CMAKE_INSTALL_PREFIX}/lib
    --prefix=${CMAKE_INSTALL_PREFIX}
    build)

set(dav1d_BUILD 
    cd build && ${dav1d_PYTHONPATH} ninja)

set(dav1d_INSTALL
    cd build && ${dav1d_PYTHONPATH} 
    ninja install ${dav1d_RENAME_TO_LIB})

ExternalProject_Add(
    dav1d
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/dav1d
    DEPENDS ${dav1d_DEPENDENCIES}
    GIT_REPOSITORY "https://code.videolan.org/videolan/dav1d.git"
    GIT_TAG ${dav1d_GIT_TAG}
    
    CONFIGURE_COMMAND ${dav1d_CONFIGURE}
    BUILD_COMMAND ${dav1d_BUILD}
    INSTALL_COMMAND ${dav1d_INSTALL}
    BUILD_IN_SOURCE 1
)

