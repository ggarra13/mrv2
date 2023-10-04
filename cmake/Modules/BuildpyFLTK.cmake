include(ExternalProject)

set(pyFLTK_SVN_REPOSITORY "https://svn.code.sf.net/p/pyfltk/code/branches/fltk1.4")
set(pyFLTK_SVN_REVISION )

set(pyFLTK_PATCH )
set(pyFLTK_CONFIGURE ${PYTHON_EXECUTABLE} setup.py swig)
set(pyFLTK_BUILD     ${PYTHON_EXECUTABLE} setup.py build)
set(pyFLTK_INSTALL   ${PYTHON_EXECUTABLE} setup.py install)

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
