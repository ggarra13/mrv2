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

    # ${CMAKE_COMMAND} -E copy_if_different
    # "${PROJECT_SOURCE_DIR}/cmake/patches/pyFLTK-patch/swig/x.i"
    # "${CMAKE_BINARY_DIR}/pyFLTK-prefix/src/pyFLTK/swig/"

set(pyFLTK_PATCH
    ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/cmake/patches/pyFLTK-patch/setup.py"
    "${CMAKE_BINARY_DIR}/pyFLTK-prefix/src/pyFLTK/")

set(pyFLTK_RUN_COMMAND )
if(WIN32)
    set(pyFLTK_RUN_COMMAND ${CMAKE_COMMAND} -E env FLTK_HOME="${FLTK_HOME}" )
endif()

set(pyFLTK_CONFIGURE ${pyFLTK_RUN_COMMAND} ${PYTHON_EXECUTABLE} setup.py swig)
set(pyFLTK_BUILD     ${pyFLTK_RUN_COMMAND} ${PYTHON_EXECUTABLE} setup.py build)
set(pyFLTK_INSTALL
    ${pyFLTK_RUN_COMMAND} ${PYTHON_EXECUTABLE} -m pip install wheel
    COMMAND ${PYTHON_EXECUTABLE} setup.py bdist_wheel
    COMMAND ${CMAKE_COMMAND} -P cmake/install_whl_files.cmake -DPYTHON_EXECUTABLE="${PYTHON_EXECUTABLE}" -DWHL_DIRECTORY="${CMAKE_BINARY_DIR}/pyFLTK-prefix/src/pyFLTK/dist")

    
ExternalProject_Add(
    pyFLTK
    SVN_REPOSITORY ${pyFLTK_SVN_REPOSITORY}
    SVN_REVISION ${pyFLTK_SVN_REVISION}
    DEPENDS ${PYTHON_DEP} ${FLTK_DEP}
    PATCH_COMMAND     ${pyFLTK_PATCH}
    CONFIGURE_COMMAND ${pyFLTK_CONFIGURE}
    BUILD_COMMAND     ${pyFLTK_BUILD}
    INSTALL_COMMAND   ${pyFLTK_INSTALL}
    BUILD_IN_SOURCE 1
)


set( pyFLTK_DEP pyFLTK )
