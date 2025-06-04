include(ExternalProject)


set(libplacebo_GIT_REPO "https://code.videolan.org/videolan/libplacebo.git")
set(libplacebo_GIT_TAG v7.349.0)

set(libplacebo_DEPS ${PYTHON_DEP})

set(libplacebo_PYTHONPATH)
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
	endif()
    elseif(WIN32)
	set(MESON_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/Bin/Scripts/meson)
    else()
	set(MESON_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/meson)
	set(libplacebo_PYTHONPATH "PYTHONPATH=${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION}:${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION}/site-packages")
    endif()
endif()

if(APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
    set(libplacebo_CFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set(libplacebo_CXXFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set(libplacebo_LDFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

set(CLANG_ENV )
if(WIN32)
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
    DEPENDS ${libplacebo_DEPS}
    CONFIGURE_COMMAND ${libplacebo_CONFIGURE}
    PATCH_COMMAND ${libplacebo_PATCH}
    BUILD_COMMAND ${libplacebo_BUILD}
    INSTALL_COMMAND ${libplacebo_INSTALL}
    BUILD_IN_SOURCE 1
)

