include(ExternalProject)


set(wayland_protocols_GIT_TAG 1.46)

set(wayland_protocols_DEPENDENCIES ${PYTHON_DEP})
message(STATUS "wayland_protocols DEPENDENCIES=${wayland_protocols_DEPENDENCIES}")

set(wayland_protocols_PYTHONPATH )
if(NOT BUILD_PYTHON)
    find_program(MESON_EXECUTABLE NAMES meson meson.exe)
    if(NOT MESON_EXECUTABLE)
	message(FATAL_ERROR "Meson build system not found!")
    endif()
else()
    if (APPLE)
	# Try to install meson via brew if not found
	execute_process(COMMAND brew install meson
            RESULT_VARIABLE BREW_RESULT
            OUTPUT_QUIET ERROR_QUIET)
	    
	if(NOT BREW_RESULT EQUAL 0)
	    message(FATAL_ERROR "Failed to install meson with Homebrew.")
	endif()

	# Try to find it again after installation
	find_program(MESON_EXECUTABLE NAMES meson
	    PATHS
	    /opt/homebrew/bin        # M1 default
	    /usr/local/bin           # Intel default
	    $ENV{PATH}
	)
	
	if (NOT MESON_EXECUTABLE)
	    message(FATAL_ERROR "meson not found in PATH")
	else()
	    message(STATUS "meson found at ${MESON_EXECUTABLE}")
	    set(wayland_protocols_PYTHONPATH "PYTHONPATH=")
	endif()
    elseif(WIN32)
	set(MESON_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/Bin/Scripts/meson)
    else()
	set(MESON_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/meson)
	set(wayland_protocols_PYTHONPATH "PYTHONPATH=${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION}:${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION}/site-packages:${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION}/lib-dynload")
    endif()
    if(NOT MESON_EXECUTABLE)
	message(FATAL_ERROR "Meson build system not found! It is in path.")
    endif()
endif()

if(APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
    set(wayland_protocols_CFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set(wayland_protocols_CXXFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set(wayland_protocols_LDFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

set(wayland_protocols_CONFIGURE
    COMMAND ${CMAKE_COMMAND} -E env
    ${CLANG_ENV}
    "CXXFLAGS=${wayland_protocols_CXXFLAGS}"
    "CFLAGS=${wayland_protocols_CFLAGS}"
    "LDFLAGS=${wayland_protocols_LDFLAGS}"
    "LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib"
    ${wayland_protocols_PYTHONPATH} 
    -- ${MESON_EXECUTABLE} setup
    --wipe
    -Dtests=true
    --prefix=${CMAKE_INSTALL_PREFIX}
    build)

set(wayland_protocols_BUILD 
    cd build && ${wayland_protocols_PYTHONPATH} ninja)

set(wayland_protocols_INSTALL
    cd build && ${wayland_protocols_PYTHONPATH} 
    ninja install)

ExternalProject_Add(
    wayland_protocols
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/wayland_protocols
    DEPENDS ${wayland_protocols_DEPENDENCIES}
    GIT_REPOSITORY "https://gitlab.freedesktop.org/wayland/wayland-protocols.git"
    GIT_TAG ${wayland_protocols_GIT_TAG}
    
    CONFIGURE_COMMAND ${wayland_protocols_CONFIGURE}
    BUILD_COMMAND ${wayland_protocols_BUILD}
    INSTALL_COMMAND ${wayland_protocols_INSTALL}
    BUILD_IN_SOURCE 1
)

set(WAYLAND_PROTOCOLS_DEP wayland_protocols)
