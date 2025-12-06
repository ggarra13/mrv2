# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Note: Linux and macOS already ship with libiconv and gettext.
#       macOS, however ships libintl only for the current version of
#       the OS.
#

if(WIN32)
    # These are relatively stable libraries, which we already
    # compile with vckpkg
    add_custom_target(
	libiconv ALL
	${CMAKE_COMMAND} -E echo "Running libiconv..."
	COMMAND ${VCPKG} install libiconv:${VCPKG_TRIPLET}
	DEPENDS ${vcpkg_DEP})
    add_custom_target(
	gettext ALL
	${CMAKE_COMMAND} -E echo "Running gettext..."
	COMMAND ${VCPKG} install gettext:${VCPKG_TRIPLET}
	DEPENDS ${vcpkg_DEP} libiconv)
    set(Gettext_DEP libiconv gettext)

    set(VCPKG_INSTALLED "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}")
    set(VCPKG_INCLUDE_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/include")
    set(VCPKG_LIB_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/lib")
    set(VCPKG_BIN_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/bin")
    
    list(APPEND CMAKE_PREFIX_PATH ${VCPKG_INSTALLED})
elseif(APPLE)
    #
    # brew .dylibs are compiled only for the current version of the OS.
    #
    # We distribute libintl dylib compatible with v10.5 on Intel macOS
    set(_libintl_h_files
	/usr/local/include/libintl.h
	/opt/homebrew/include/libintl.h)
    foreach(_h_file ${_libintl_h_files})
	if (EXISTS "${_h_file})")
	    message(STATUS "Copying ${_h_file}")
	    file(COPY "${_h_file}" DESTINATION ${CMAKE_INSTALL_PREFIX}/include/)
	endif()
    endforeach()
    
    set(_libintl_dylib_files
	/usr/local/lib/libintl.dylib
	/opt/homebrew/lib/libintl.dylib
    )
    foreach(_dylib_file ${_libintl_dylib_files})
	if (EXISTS "${_dylib_file}")
	    message(STATUS "Copying ${_dylib_file}")
	    file(COPY "${_dylib_file}" DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/
		FOLLOW_SYMLINK_CHAIN)
	endif()
    endforeach()    
endif()
