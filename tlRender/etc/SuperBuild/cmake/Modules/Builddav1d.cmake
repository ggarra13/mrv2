include(ExternalProject)


set(dav1d_GIT_TAG 1.3.0)

set(dav1d_DEPS ${PYTHON_DEP})
if(UNIX)
    set(dav1d_DEPS NASM ${dav1d_DEPS})
endif()

set(dav1d_PYTHONPATH )
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
	set(dav1d_PYTHONPATH "PYTHONPATH=${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION}:${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION}/site-packages")
    endif()
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
    DEPENDS ${dav1d_DEPS}
    GIT_REPOSITORY "https://code.videolan.org/videolan/dav1d.git"
    GIT_TAG ${dav1d_GIT_TAG}
    
    CONFIGURE_COMMAND ${dav1d_CONFIGURE}
    BUILD_COMMAND ${dav1d_BUILD}
    INSTALL_COMMAND ${dav1d_INSTALL}
    BUILD_IN_SOURCE 1
)

