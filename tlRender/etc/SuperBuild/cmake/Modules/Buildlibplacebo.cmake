include(ExternalProject)

include(functions/find_meson_executable)
find_meson_executable(libplacebo)



set(libplacebo_GIT_REPO "https://code.videolan.org/videolan/libplacebo.git")
set(libplacebo_GIT_TAG v7.351.0) # was v7.349.0

set(libplacebo_DEPENDENCIES ${PYTHON_DEP})
message(STATUS "libplacebo DEPENDENCIES=${libplacebo_DEPENDENCIES}")

if(APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
    set(libplacebo_CFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set(libplacebo_CXXFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set(libplacebo_LDFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

set(CLANG_ENV )
if(WIN32)
    # \@todo: Replace clang with GENERIC_COMPILER_NAME
    set(CLANG_ENV CC=clang CXX=clang)
endif()

if(UNIX)
    set(libplacebo_LDFLAGS -lstdc++)  # \@bug: in Rocky Linux 8.10+
endif()

set(libplacebo_CONFIGURE
    COMMAND git submodule update --init
    COMMAND ${CMAKE_COMMAND} -E env
    ${CLANG_ENV}
    "CXXFLAGS=${libplacebo_CXXFLAGS}"
    "CFLAGS=${libplacebo_CFLAGS}"
    "LDFLAGS=${libplacebo_LDFLAGS}"
    "LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib"
    ${libplacebo_PYTHONPATH}
    --
    ${MESON_EXECUTABLE} setup
    --wipe
    -Dvulkan=disabled
    -Ddemos=false
    -Dshaderc=disabled
    -Dlcms=disabled
    -Dglslang=disabled
    -Dlibdir=${CMAKE_INSTALL_PREFIX}/lib
    --prefix=${CMAKE_INSTALL_PREFIX}
    build)

set(libplacebo_BUILD
    cd build && ${libplacebo_PYTHONPATH} ninja)

set(libplacebo_INSTALL
    cd build && ${libplacebo_PYTHONPATH} ninja install)

set(libplacebo_PATCH)


ExternalProject_Add(
    libplacebo
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libplacebo
    GIT_REPOSITORY ${libplacebo_GIT_REPO}
    GIT_TAG ${libplacebo_GIT_TAG}
    DEPENDS ${libplacebo_DEPENDENCIES}
    CONFIGURE_COMMAND ${libplacebo_CONFIGURE}
    PATCH_COMMAND ${libplacebo_PATCH}
    BUILD_COMMAND ${libplacebo_BUILD}
    INSTALL_COMMAND ${libplacebo_INSTALL}
    BUILD_IN_SOURCE 1
)

