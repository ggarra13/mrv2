include(ExternalProject)

set(OTIO_GIT_REPOSITORY "https://github.com/PixarAnimationStudios/OpenTimelineIO.git")
set(OTIO_GIT_TAG "v0.18.1")

set(OTIO_DEPENDENCIES ${PYTHON_DEP} Imath)
message(STATUS "OTIO DEPENDENCIES=${OTIO_DEPENDENCIES}")

set(OTIO_SHARED_LIBS ON)
if(NOT BUILD_SHARED_LIBS)
    set(OTIO_SHARED_LIBS OFF)
endif()


set(OTIO_PYTHON_INSTALL ON)
set(OTIO_INSTALL_PYTHON_MODULES ON)

if(NOT BUILD_PYTHON)
    set(OTIO_PYTHON_INSTALL OFF)
    set(OTIO_INSTALL_PYTHON_MODULES OFF)
endif()

set(OTIO_PATCH "")
if(WIN32)
    set(OTIO_PATCH ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/patches/OTIO-patch/src/opentime/timeRange.h
	${CMAKE_CURRENT_BINARY_DIR}/OTIO/src/OTIO/src/opentime/timeRange.h
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/patches/OTIO-patch/src/opentimelineio/errorStatus.h
	${CMAKE_CURRENT_BINARY_DIR}/OTIO/src/OTIO/src/opentimelineio/errorStatus.h)

endif()

# Derive these from your Python build variables
if(WIN32)
    set(_Python_ROOT ${CMAKE_INSTALL_PREFIX})
    set(_Python_EXE  ${CMAKE_INSTALL_PREFIX}/bin/python.exe)
endif()

set(OTIO_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DBUILD_TESTING=OFF
    -DOTIO_FIND_IMATH=ON
    
    -DPYBIND11_FINDPYTHON=ON
    -DPYBIND11_NOPYTHON=OFF
    -DPYBIND11_NUMPY_1_ONLY=OFF
    -DPYBIND11_SIMPLE_GIL_MANAGEMENT=OFF
    -DPYBIND11_USE_CROSSCOMPILING=OFF
    -DPYBIND11_TEST=OFF
    
    # --- Pin Python for pybind11 / FindPython ---
    -DPython_ROOT_DIR=${_Python_ROOT}
    -DPython_EXECUTABLE=${_Python_EXE}
    -DPython3_ROOT_DIR=${_Python_ROOT}
    -DPython3_EXECUTABLE=${_Python_EXE}

    # Tell FindPython to trust ROOT_DIR and not search elsewhere
    -DPython_FIND_STRATEGY=LOCATION
    -DPython3_FIND_STRATEGY=LOCATION
    -DPython_FIND_REGISTRY=NEVER          # <-- stops x64 registry hit
    -DPython3_FIND_REGISTRY=NEVER

    -DPython_FIND_VIRTUALENV=NEVER
    -DPython3_FIND_VIRTUALENV=NEVER

    # Explicit architecture guard
    -DCMAKE_GENERATOR_PLATFORM=ARM64     # if not already set globally
    
    -DOTIO_INSTALL_COMMANDLINE_TOOLS=OFF
    -DOTIO_SHARED_LIBS=${OTIO_SHARED_LIBS}
    -DOTIO_PYTHON_INSTALL=${OTIO_PYTHON_INSTALL}
    -DOTIO_INSTALL_PYTHON_MODULES=${OTIO_INSTALL_PYTHON_MODULES})

ExternalProject_Add(
    OTIO
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OTIO
    DEPENDS ${OTIO_DEPENDENCIES}
    GIT_REPOSITORY ${OTIO_GIT_REPOSITORY}
    GIT_TAG ${OTIO_GIT_TAG}
    PATCH_COMMAND ${OTIO_PATCH}
    LIST_SEPARATOR |
    CMAKE_ARGS ${OTIO_ARGS})
