# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

include(ExternalProject)

set(pyFLTK_SVN_REPOSITORY "https://svn.code.sf.net/p/pyfltk/code/branches/fltk1.4")
set(pyFLTK_SVN_REVISION 658)
set(pyFLTK_SVN_REVISION_ARG -r ${pyFLTK_SVN_REVISION})

if(NOT PYTHON_EXECUTABLE)
    if(UNIX)
	set(PYTHON_EXECUTABLE python3)
    else()
	set(PYTHON_EXECUTABLE python)
    endif()
endif()



#
# Environment setup
#
set(pyFLTK_CXX_FLAGS ${CMAKE_CXX_FLAGS} )

set(pyFLTK_OLD_LD_LIBRARY_PATH $ENV{OLD_LD_LIBRARY_PATH})
set(pyFLTK_OLD_DYLD_LIBRARY_PATH $ENV{OLD_DYLD_LIBRARY_PATH})

set(pyFLTK_LD_LIBRARY_PATH $ENV{LD_LIBRARY_PATH})
set(pyFLTK_DYLD_LIBRARY_PATH $ENV{DYLD_LIBRARY_PATH})

set(pyFLTK_PATH $ENV{PATH})
if(WIN32)
    string(REPLACE ";" "|" pyFLTK_PATH "$ENV{PATH}")
endif()

#
# Handle macOS min version.
#
if(APPLE)
    if(CMAKE_OSX_DEPLOYMENT_TARGET)
	set(pyFLTK_CXX_FLAGS "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} ${pyFLTK_CXX_FLAGS}")
    endif()
endif()


#
# Old environment and checkout command
#
if(WIN32)
    set(pyFLTK_OLD_ENV ${CMAKE_COMMAND} -E env -- )
    set(pyFLTK_CHECKOUT_CMD ${pyFLTK_OLD_ENV} svn checkout ${pyFLTK_SVN_REVISION_ARG} ${pyFLTK_SVN_REPOSITORY} pyFLTK)
elseif(APPLE)
    set(pyFLTK_OLD_ENV ${CMAKE_COMMAND} -E env "DYLD_LIBRARY_PATH=${pyFLTK_OLD_DYLD_LIBRARY_PATH}" -- )
    set(pyFLTK_CHECKOUT_CMD ${pyFLTK_OLD_ENV} svn checkout ${pyFLTK_SVN_REVISION_ARG} ${pyFLTK_SVN_REPOSITORY} pyFLTK)
else()
    set(pyFLTK_OLD_ENV ${CMAKE_COMMAND} -E env "LD_LIBRARY_PATH=${pyFLTK_OLD_LD_LIBRARY_PATH}" -- )
    set(pyFLTK_CHECKOUT_CMD ${pyFLTK_OLD_ENV} svn checkout ${pyFLTK_SVN_REVISION_ARG} ${pyFLTK_SVN_REPOSITORY} pyFLTK)
endif()



#
# Commands
#
set(pyFLTK_PATCH
    # For avoiding the show(argv) messing mrv2's color pallette.
    COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/cmake/patches/pyFLTK-patch/swig/WindowShowTypemap.i"
    "${CMAKE_BINARY_DIR}/deps/pyFLTK/src/pyFLTK/swig/"
)

# Environment setup for configure, building and installing
set(pyFLTK_ENV ${CMAKE_COMMAND} -E env CXXFLAGS=${pyFLTK_CXX_FLAGS} )
if(WIN32)
    set(pyFLTK_ENV ${pyFLTK_ENV} "PATH=${pyFLTK_PATH}" FLTK_HOME=${CMAKE_INSTALL_PREFIX} --) 
elseif(APPLE)
    set(pyFLTK_ENV ${pyFLTK_ENV} "PATH=${pyFLTK_PATH}" DYLD_LIBRARY_PATH=${pyFLTK_DYLD_LIBRARY_PATH} -- )
else()
    set(pyFLTK_ENV ${pyFLTK_ENV} "PATH=${pyFLTK_PATH}" LD_LIBRARY_PATH=${pyFLTK_LD_LIBRARY_PATH} -- )
endif()

set(pyFLTK_DEBUG )
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(pyFLTK_DEBUG --debug )
endif()

if (NOT ${Python_VERSION})
    message(FATAL_ERROR "Python_VERSION not defined: ${Python_VERSION}")
endif()

# Commands for configure, build and install
set(pyFLTK_CONFIGURE
    COMMAND ${pyFLTK_ENV} ${PYTHON_EXECUTABLE} -m pip install setuptools
    COMMAND ${pyFLTK_ENV} ${PYTHON_EXECUTABLE} setup.py swig --enable-shared ${pyFLTK_DEBUG})
set(pyFLTK_BUILD     ${pyFLTK_ENV} ${PYTHON_EXECUTABLE} setup.py build --enable-shared ${pyFLTK_DEBUG})
set(pyFLTK_INSTALL ${pyFLTK_ENV} ${PYTHON_EXECUTABLE} -m pip install . )

ExternalProject_Add(
    pyFLTK
    # \bug: subversion on Linux is usually not compiled with the latest OpenSSL
    #       so we need to DOWNLOAD_COMMAND for checking out the repository.
    # SVN_REPOSITORY ${pyFLTK_SVN_REPOSITORY}
    # SVN_REVISION ${pyFLTK_SVN_REVISION}
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/deps/pyFLTK
    DEPENDS ${PYTHON_DEP} ${FLTK_DEP}
    DOWNLOAD_COMMAND  "${pyFLTK_CHECKOUT_CMD}"
    PATCH_COMMAND     ${pyFLTK_PATCH}
    CONFIGURE_COMMAND "${pyFLTK_CONFIGURE}"
    BUILD_COMMAND     "${pyFLTK_BUILD}"
    INSTALL_COMMAND   "${pyFLTK_INSTALL}"
    LIST_SEPARATOR |
    BUILD_IN_SOURCE 1
)


set( pyFLTK_DEP pyFLTK )
