# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

# message( STATUS "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}" )
# message( STATUS "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}" )
# message( STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}" )

#
# bug:
#
# According to the CMAKE docs CMAKE_INSTALL_PREFIX should point to the
# install directory, but on Linux, macOS and Windows each path is different.
#
if ( UNIX AND NOT APPLE )
    set( ROOT_DIR "${CMAKE_CURRENT_BINARY_DIR}/../../../../../../../../../" )
elseif(APPLE)
    set( ROOT_DIR "${CMAKE_INSTALL_PREFIX}/../../../../../../../../../../../../" )
else()
    set( ROOT_DIR "${CMAKE_INSTALL_PREFIX}/../../../../../../../../../" )
endif()

file(REAL_PATH ${ROOT_DIR} ROOT_DIR )


message( STATUS "cmake/prepackage.cmake has ROOT_DIR=${ROOT_DIR}" )

include( "${ROOT_DIR}/cmake/functions.cmake" )


if( UNIX AND NOT APPLE )
    #
    # @bug: Linux CMAKE_INSTALL_PREFIX is broken and not pointing to
    #       pre-packaging directory!!!
    #
    if (EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}")
	set( CPACK_PREPACKAGE "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}" )
    else()
	set( CPACK_PREPACKAGE "${CMAKE_INSTALL_PREFIX}" )
    endif()
else()
    set( CPACK_PREPACKAGE "${CMAKE_INSTALL_PREFIX}" )
endif()

message( STATUS "CPACK_PREPACKAGE=${CPACK_PREPACKAGE}" )

#
# Remove .a, .lib and .dll files from packaging lib/ directory
#
file( GLOB STATIC_LIBS "${CPACK_PREPACKAGE}/lib/*.a"
		       "${CPACK_PREPACKAGE}/lib/*.lib"
		       "${CPACK_PREPACKAGE}/lib/*.dll" )

if ( NOT "${STATIC_LIBS}" STREQUAL "" )
    file( REMOVE ${STATIC_LIBS} )
endif()


#
# Remove include files from packaging directory
#
file( REMOVE_RECURSE "${CPACK_PREPACKAGE}/include" )

#
# Install system .SO dependencies
#
if( UNIX)

    set(linux_lib64_dir "${CPACK_PREPACKAGE}/lib64")
    if (EXISTS "${linux_lib64_dir}" )
	# For pyFLTK we need to install all libfltk DSOs including those we
	# do not use, like forms.
	message( NOTICE "${linux_lib64_dir} exists...")
	file(GLOB fltk_dsos "${linux_lib64_dir}/libfltk*.so")
	file(INSTALL
	    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
	    TYPE SHARED_LIBRARY
	    FOLLOW_SYMLINK_CHAIN
	    FILES ${fltk_dsos}
	)
    else()
	message( NOTICE "${linux_lib64_dir} does not exist...")
    endif()
	
    #
    # Glob python directories as we don't know the version of python in this
    # script, as it does not inherit variables from the main CMakeLists.
    #
    file(GLOB MRV2_PYTHON_LIB_DIRS "${CPACK_PREPACKAGE}/lib/python*")

    #
    # Grab the last version, in case there are several
    #
    list(GET MRV2_PYTHON_LIB_DIRS -1 MRV2_PYTHON_LIB_DIR)

    
    set(MRV2_PYTHON_SITE_PACKAGES_DIR "${MRV2_PYTHON_LIB_DIR}/site-packages")
	
    set( MRV2_EXES "${CPACK_PREPACKAGE}/bin/mrv2" )
    
    if ( APPLE )
	#
	# Get DYLIB dependencies of componenets
	#
	get_macos_runtime_dependencies( "${MRV2_EXES}" DEPENDENCIES )
    else()

	
	#
	# We need to get the dependencies of the python DSOs to avoid
	# issues like openssl and libcrypto changing between Rocky Linux 8.1
	# and Ubuntu 22.04.5.
	#
	set(MRV2_PYTHON_LIB_DIR "${MRV2_PYTHON_LIB_DIR}/lib-dynload")
	file(GLOB python_dsos "${MRV2_PYTHON_LIB_DIR}/*.so")
	list(APPEND MRV2_EXES ${python_dsos} )

	#
	# Get DSO dependencies of componenets
	#
	get_runtime_dependencies( "${MRV2_EXES}" DEPENDENCIES )
    endif()
    file( COPY ${DEPENDENCIES} DESTINATION "${CPACK_PREPACKAGE}/lib/" )
elseif(WIN32)
    #
    # Remove usd directory from lib/ directory on Windows
    #
    file( REMOVE_RECURSE "${CPACK_PREPACKAGE}/lib/usd" )
    
    #
    # Set python's site-packages dir for .exe installer.
    #
    # When building an .exe installer on Windows, the site-packages will
    # be inside an applications component directory.
    #
    set(MRV2_PYTHON_APP_DIR "${CPACK_PREPACKAGE}/applications")
    set(MRV2_PYTHON_APP_LIB_DIR "${MRV2_PYTHON_APP_DIR}/bin/Lib/")
    set(MRV2_PYTHON_SITE_PACKAGES_DIR
	"${MRV2_PYTHON_APP_LIB_DIR}/site-packages")
    
    #
    # Don't pack sphinx and other auxiliary documentation libs
    #
    file(GLOB MRV2_UNUSED_PYTHON_DIRS
    "${MRV2_PYTHON_APP_LIB_DIR}/test*"
    "${MRV2_PYTHON_APP_LIB_DIR}/ctypes/test*"
    "${MRV2_PYTHON_APP_LIB_DIR}/distutils/test*"
    "${MRV2_PYTHON_APP_LIB_DIR}/idlelib/idle_test*"
    "${MRV2_PYTHON_APP_LIB_DIR}/lib2to3/test*"
    "${MRV2_PYTHON_APP_LIB_DIR}/sqlite3/test*"
    "${MRV2_PYTHON_APP_LIB_DIR}/tkinter/test*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/alabaster*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/babel*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/colorama*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/docutils*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/imagesize*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/Jinja*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/MarkupSafe*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/Pygments*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/snowballstemmer*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/sphinx*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/unittest*")

	
    if ( NOT "${MRV2_UNUSED_PYTHON_DIRS}" STREQUAL "" )
	file( REMOVE_RECURSE ${MRV2_UNUSED_PYTHON_DIRS} )
    endif()

    #
    # Set python's site-packages dir for .zip.
    #
    # When building an .exe on Windows, the site-packages will
    # be inside an application directory.
    #
    set(MRV2_PYTHON_SITE_PACKAGES_DIR
	"${CPACK_PREPACKAGE}/bin/Lib/site-packages")
endif()


#
# Don't pack sphinx and other auxiliary documentation libs
#
file(GLOB MRV2_UNUSED_PYTHON_DIRS
    "${MRV2_PYTHON_LIB_DIR}/test*"
    "${MRV2_PYTHON_LIB_DIR}/ctypes/test*"
    "${MRV2_PYTHON_LIB_DIR}/distutils/test*"
    "${MRV2_PYTHON_LIB_DIR}/idlelib/idle_test*"
    "${MRV2_PYTHON_LIB_DIR}/lib2to3/test*"
    "${MRV2_PYTHON_LIB_DIR}/sqlite3/test*"
    "${MRV2_PYTHON_LIB_DIR}/tkinter/test*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/alabaster*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/babel*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/colorama*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/docutils*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/imagesize*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/Jinja*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/MarkupSafe*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/Pygments*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/snowballstemmer*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/sphinx*"
    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/unittest*")

	
if ( NOT "${MRV2_UNUSED_PYTHON_DIRS}" STREQUAL "" )
    file( REMOVE_RECURSE ${MRV2_UNUSED_PYTHON_DIRS} )
endif()
