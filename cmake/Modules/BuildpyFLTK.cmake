# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

include(ExternalProject)

set(pyFLTK_SVN_REPOSITORY "https://svn.code.sf.net/p/pyfltk/code/branches/fltk1.4")
set(pyFLTK_SVN_REVISION 627)
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
set(pyFLTK_LD_LIBRARY_PATH $ENV{OLD_LD_LIBRARY_PATH})


set(pyFLTK_ENV ${CMAKE_COMMAND} -E env FLTK_HOME=${CMAKE_INSTALL_PREFIX} CXXFLAGS=${pyFLTK_CXX_FLAGS} -- )

if(WIN32)
    set(pyFLTK_CHECKOUT_CMD ${CMAKE_COMMAND} -E env -- svn checkout ${pyFLTK_SVN_REVISION_ARG} ${pyFLTK_SVN_REPOSITORY} pyFLTK)
elseif(APPLE)
    set(pyFLTK_DYLD_LIBRARY_PATH $ENV{OLD_DYLD_LIBRARY_PATH})
    set(pyFLTK_CHECKOUT_CMD  ${CMAKE_COMMAND} -E env DYLD_LIBRARY_PATH=${pyFLTK_DYLD_LIBRARY_PATH} -- svn checkout ${pyFLTK_SVN_REVISION_ARG} ${pyFLTK_SVN_REPOSITORY} pyFLTK)
    if(CMAKE_OSX_DEPLOYMENT_TARGET)
	list(APPEND pyFLTK_CXX_FLAGS "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()
else()
    set(pyFLTK_CHECKOUT_CMD ${CMAKE_COMMAND} -E env LD_LIBRARY_PATH=${pyFLTK_LD_LIBRARY_PATH} svn checkout ${pyFLTK_SVN_REVISION_ARG} ${pyFLTK_SVN_REPOSITORY} pyFLTK)
endif()



#
# Commands
#
set(pyFLTK_PATCH
    COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/cmake/patches/pyFLTK-patch/setup.py"
    "${CMAKE_BINARY_DIR}/pyFLTK-prefix/src/pyFLTK/"
    COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/cmake/patches/pyFLTK-patch/swig/WindowShowTypemap.i"
    "${CMAKE_BINARY_DIR}/pyFLTK-prefix/src/pyFLTK/swig/")
set(pyFLTK_CONFIGURE ${pyFLTK_ENV} ${PYTHON_EXECUTABLE} setup.py swig --enable-shared)
set(pyFLTK_BUILD     ${pyFLTK_ENV} ${PYTHON_EXECUTABLE} setup.py build --enable-shared)
set(pyFLTK_INSTALL ${pyFLTK_ENV} ${PYTHON_EXECUTABLE} setup.py install --enable-shared)




ExternalProject_Add(
    pyFLTK
    # \bug: subversion on Linux is usually not compiled with the latest OpenSSL
    #       so we need to DOWNLOAD_COMMAND for checking out the repository.
    # SVN_REPOSITORY ${pyFLTK_SVN_REPOSITORY}
    # SVN_REVISION ${pyFLTK_SVN_REVISION}
    DEPENDS ${PYTHON_DEP} ${FLTK_DEP} ${SVN_DEP}
    DOWNLOAD_COMMAND  ${pyFLTK_CHECKOUT_CMD}
    PATCH_COMMAND     ${pyFLTK_PATCH}
    CONFIGURE_COMMAND "${pyFLTK_CONFIGURE}"
    BUILD_COMMAND     "${pyFLTK_BUILD}"
    INSTALL_COMMAND   ${pyFLTK_INSTALL}
    BUILD_IN_SOURCE 1
)


set( pyFLTK_DEP pyFLTK )
