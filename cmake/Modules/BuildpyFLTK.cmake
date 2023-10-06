# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

include(ExternalProject)

set(pyFLTK_SVN_REPOSITORY "https://svn.code.sf.net/p/pyfltk/code/branches/fltk1.4")
set(pyFLTK_SVN_REVISION )

if(NOT PYTHON_EXECUTABLE)
    if(UNIX)
	set(PYTHON_EXECUTABLE python3)
    else()
	set(PYTHON_EXECUTABLE python)
    endif()
endif()

set(pyFLTK_PATCH
    ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/cmake/patches/pyFLTK-patch/setup.py"
    "${CMAKE_BINARY_DIR}/pyFLTK-prefix/src/pyFLTK/")

set(pyFLTK_ENV )
if(WIN32)
    set(pyFLTK_ENV ${CMAKE_COMMAND} -E env FLTK_HOME=${FLTK_HOME} )
endif()


set(pyFLTK_CONFIGURE ${pyFLTK_ENV} ${PYTHON_EXECUTABLE} setup.py swig)
set(pyFLTK_BUILD     ${pyFLTK_ENV} ${PYTHON_EXECUTABLE} setup.py build)
set(pyFLTK_INSTALL   ${PYTHON_EXECUTABLE} -m pip install wheel )
set(pyFLTK_WHEEL     ${pyFLTK_ENV} ${PYTHON_EXECUTABLE} setup.py bdist_wheel)
set(pyFLTK_INSTALL_WHEEL ${CMAKE_COMMAND}
    -DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}
    -DWHL_DIRECTORY=${CMAKE_BINARY_DIR}/pyFLTK-prefix/src/pyFLTK/dist
    -P "${CMAKE_SOURCE_DIR}/cmake/install_whl_files.cmake" )
    
ExternalProject_Add(
    pyFLTK
    SVN_REPOSITORY ${pyFLTK_SVN_REPOSITORY}
    SVN_REVISION ${pyFLTK_SVN_REVISION}
    DEPENDS ${PYTHON_DEP} ${FLTK_DEP}
    PATCH_COMMAND     "${pyFLTK_PATCH}"
    CONFIGURE_COMMAND "${pyFLTK_CONFIGURE}"
    BUILD_COMMAND     "${pyFLTK_BUILD}"
    INSTALL_COMMAND   "${pyFLTK_INSTALL}"
    COMMAND "${pyFLTK_WHEEL}"
    COMMAND "${pyFLTK_INSTALL_WHEEL}"
    BUILD_IN_SOURCE 1
)
    
set( pyFLTK_DEP pyFLTK )
