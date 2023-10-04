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

message(STATUS "pyFLTK got PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}" )

set(pyFLTK_PATCH )
set(pyFLTK_CONFIGURE ${PYTHON_EXECUTABLE} setup.py --disable-forms swig)
set(pyFLTK_BUILD     ${PYTHON_EXECUTABLE} setup.py --disable-forms build)
set(pyFLTK_INSTALL   ${PYTHON_EXECUTABLE} setup.py --disable-forms install)

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
