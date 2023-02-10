# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Function used to take .fl files (GUI creator into .cxx / .h files.
#
function (FLTK_RUN_FLUID TARGET SOURCES)
    set (CXX_FILES)
    message( "CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}" )
    foreach (src ${SOURCES})
	if ("${src}" MATCHES "\\.fl$")
	    string(REGEX REPLACE "(.*/)?(.*).fl" \\2 basename ${src})
	    message( "Running: cd ${CMAKE_CURRENT_BINARY_DIR} &&  ${FLTK_FLUID_EXECUTABLE} -c ${CMAKE_CURRENT_SOURCE_DIR}/${src}" )
	    add_custom_command(
		OUTPUT "${basename}.cxx" "${basename}.h"
		COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && ${FLTK_FLUID_EXECUTABLE} -c "${CMAKE_CURRENT_SOURCE_DIR}/${src}"
		DEPENDS ${src}
		MAIN_DEPENDENCY ${src}
		)
	    set( _cxx_file "${CMAKE_CURRENT_BINARY_DIR}/${basename}.cxx" )
	    if ( EXISTS ${_cxx_file} )
		message( "FLUID: Created ${_cxx_file}... " )
	    else()
		message( FATAL_ERROR "FLUID did not create ${_cxx_file}." )
	    endif()
	    list (APPEND CXX_FILES ${_cxx_file} )
	endif ("${src}" MATCHES "\\.fl$")
    endforeach ()
    set (${TARGET} ${CXX_FILES} PARENT_SCOPE)
endfunction()

#
# Function used to discard system DSOS or those already insta
#
function( is_system_lib TARGET ISSYSLIB )
    set( _acceptedlibs libmd )
    set( _syslibs libOpenGL libGL libEGL libGLdispatch libGLX libX nvidia libdrm2 libpthread libresolv libm librt libdl libxcb libasound libgpg-error libfontconfig libfreetype libxshmfence libc libstdc libgcc_s libselinux ld-linux )
    set( ${ISSYSLIB} 0 PARENT_SCOPE)
    foreach( lib ${_acceptedlibs} )
	if ("${TARGET}" MATCHES "${lib}")
	    return()
	endif()
    endforeach()
    foreach( lib ${_syslibs} )
	if ("${TARGET}" MATCHES "${lib}")
	    set( ${ISSYSLIB} 1 PARENT_SCOPE)
	    break()
	endif()
	if ("${TARGET}" MATCHES "${CMAKE_INSTALL_PREFIX}/lib")
	    set( ${ISSYSLIB} 1 PARENT_SCOPE)
	    break()
	endif()
    endforeach()

endfunction()

#
# Function used to install a library  with all .so dependencies
#
function(install_library_with_deps LIBRARY)

    is_system_lib (${LIBRARY} sys_lib)
    if ( ${sys_lib} EQUAL 1 )
	message( STATUS "SKIPPED installing ${LIBRARY}" )
	return()
    endif()

    file(INSTALL
	DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
	TYPE SHARED_LIBRARY
	FOLLOW_SYMLINK_CHAIN
	FILES "${LIBRARY}"
	)
    file(GET_RUNTIME_DEPENDENCIES
	LIBRARIES ${LIBRARY}
	RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPS
	UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPS
	)
    foreach(FILE ${RESOLVED_DEPS})
	if(NOT IS_SYMLINK ${FILE})
	    install_library_with_deps(${FILE})
	endif()
    endforeach()
    foreach(FILE ${UNRESOLVED_DEPS})
	message(STATUS "Unresolved from ${LIBRARY}: ${FILE}")
    endforeach()
endfunction()

#
# Function used to get runtime dependencies as cmake GET_RUNTIME_DEPENDENCIES is
# broken.
#
function( get_runtime_dependencies TARGET DEPENDENCIES )

    # Add CMAKE_INSTALL_PREFIX first to library path
    set( ENV{LD_LIBRARY_PATH} "${CMAKE_INSTALL_PREFIX}/lib:${LD_LIBRARY_PATH}" )


    foreach (exe "${TARGET}")
	if ( EXISTS ${exe} )
	    message( STATUS "PARSING ${exe} for DSOs...." )
	    execute_process(COMMAND ldd ${exe} OUTPUT_VARIABLE ldd_out)
	    string (REPLACE "\n" ";" ldd_out_lines ${ldd_out})
	    foreach (line ${ldd_out_lines})
		string (REGEX REPLACE "^.* => | \(.*\)" "" pruned ${line})
		string (STRIP ${pruned} dep_filename)
		if (IS_ABSOLUTE ${dep_filename})
		    is_system_lib (${dep_filename} sys_lib)
		    if (sys_lib EQUAL 0 OR INSTALL_SYSLIBS STREQUAL "true")
			list (FIND dependencies ${dep_filename} found)
			if (found LESS 0)
			    message( STATUS "${dep_filename} must be installed" )
			    install_library_with_deps( ${dep_filename} )
			endif()
		    endif()
		endif()
	    endforeach()
	else()
	    message( WARNING "Executable ${exe} does not exist!" )
	endif()
    endforeach()
endfunction()




#
# Macro used to turn a list of .cpp/.h files into an absolute path
#
macro( files_to_absolute_paths )
    set( PO_FILES )
    foreach( filename ${SOURCES} ${HEADERS} )
	file(REAL_PATH ${filename} ABS_FILE )
	set( PO_FILES ${ABS_FILE} ${PO_FILES} )
    endforeach()
endmacro()
