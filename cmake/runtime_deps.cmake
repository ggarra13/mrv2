# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer 2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This does not work as it picks DSOs twice from /usr/local/Lib and
# CMAKE_INSTALL_PREFIX/lib
#
# Use function get_runtime_dependencies defined in functions.cmake instead.
#

set(MY_DEPENDENCY_PATHS /usr/lib/x86_64-linux-gnu/ )

# Transfer the value of ${MY_DEPENDENCY_PATHS} into the install script
install(CODE "set(MY_DEPENDENCY_PATHS \"${MY_DEPENDENCY_PATHS}\")")
install(CODE [[
    file(GET_RUNTIME_DEPENDENCIES
	EXECUTABLES $<TARGET_FILE:mrv2>
	RESOLVED_DEPENDENCIES_VAR _r_deps
	UNRESOLVED_DEPENDENCIES_VAR _u_deps
	DIRECTORIES ${MY_DEPENDENCY_PATHS}
	)
    foreach(_file ${_r_deps})
	message( STATUS "Processing ${_file}" )
	file(INSTALL
	    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
	    TYPE SHARED_LIBRARY
	    FOLLOW_SYMLINK_CHAIN
	    FILES "${_file}"
	    )
    endforeach()
    list(LENGTH _u_deps _u_length)
    if("${_u_length}" GREATER 0)
	message(WARNING "Unresolved dependencies detected!")
    endif()
    ]])
