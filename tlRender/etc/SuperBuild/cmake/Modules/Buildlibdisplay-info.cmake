include(ExternalProject)

include(functions/find_meson_executable)
find_meson_executable(libdisplay_info)

set(libdisplay_info_GIT_REPO
    "https://gitlab.freedesktop.org/emersion/libdisplay-info.git")
set(libdisplay_info_GIT_TAG 0.3.0)

set(libdisplay_info_DEPENDENCIES ${PYTHON_DEP})

message(STATUS "libdisplay_info DEPENDENCIES=${libdisplay_info_DEPENDENCIES}")

if(APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
    set(libdisplay_info_CFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set(libdisplay_info_CXXFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set(libdisplay_info_LDFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

set(libdisplay_info_CONFIGURE
    COMMAND ${CMAKE_COMMAND} -E env
    "CXXFLAGS=${libdisplay_info_CXXFLAGS}"
    "CFLAGS=${libdisplay_info_CFLAGS}"
    "LDFLAGS=${libdisplay_info_LDFLAGS}"
    "LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib"
    ${libdisplay_info_PYTHONPATH} 
    -- ${MESON_EXECUTABLE} setup
    --wipe
    --prefix=${CMAKE_INSTALL_PREFIX}
    build)

set(libdisplay_info_BUILD 
    cd build && ${libdisplay_info_PYTHONPATH} ninja)

set(libdisplay_info_INSTALL
    cd build && ${libdisplay_info_PYTHONPATH} 
    ninja install)

ExternalProject_Add(
    libdisplay-info
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libdisplay_info
    DEPENDS ${libdisplay_info_DEPENDENCIES}
    GIT_REPOSITORY ${libdisplay_info_GIT_REPO}
    GIT_TAG ${libdisplay_info_GIT_TAG}
    
    CONFIGURE_COMMAND ${libdisplay_info_CONFIGURE}
    BUILD_COMMAND ${libdisplay_info_BUILD}
    INSTALL_COMMAND ${libdisplay_info_INSTALL}
    BUILD_IN_SOURCE 1
)

