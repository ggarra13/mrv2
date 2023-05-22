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
function( is_macos_system_lib TARGET ISSYSLIB )


    set( ${ISSYSLIB} 0 PARENT_SCOPE)

    if ("${TARGET}" MATCHES "/mrv2")
        # local library
	set( ${ISSYSLIB} 1 PARENT_SCOPE)
	return()
    endif()
    
    if ("${TARGET}" MATCHES "^/System")
	set( ${ISSYSLIB} 1 PARENT_SCOPE)
        return()
    endif()
    
    if ("${TARGET}" MATCHES "^/usr/lib/")
	set( ${ISSYSLIB} 1 PARENT_SCOPE)
        return()
    endif()

endfunction()
#
# Function used to discard system DSOS or those already insta
#
function( is_system_lib TARGET ISSYSLIB )

    #
    # List of libraries that are accepted to distribute
    #
    set( _acceptedlibs libmount libmd libcairo libcap libcrypto )

    #
    # List of system libraries that should not be distributed
    #
    set( _syslibs ld-linux libasound libc libdl libharfbuzz libfontconfig libfreetype libgcc_s libgpg-error libEGL libGL libGLdispatch libGLX libX nvidia libdrm2 libpthread libresolv libm libOpenGL libpulse libpulse-simple librt libxcb libxshmfence libstdc libz )

    
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
# Function used to get runtime dependencies as cmake GET_RUNTIME_DEPENDENCIES is
# broken.
#
function( get_macos_runtime_dependencies TARGET DEPENDENCIES )

    # Add CMAKE_INSTALL_PREFIX first to library path
    set( ENV{DYLD_LIBRARY_PATH} "${CMAKE_INSTALL_PREFIX}/lib:${DYLD_LIBRARY_PATH}" )

    set(DEPENDENCIES  )
    foreach (exe "${TARGET}")
	if ( EXISTS ${exe} )
	    message( STATUS "PARSING ${exe} for DSOs...." )
	    execute_process(COMMAND otool -L ${exe} OUTPUT_VARIABLE ldd_out)
	    string (REPLACE "\n" ";" ldd_out_lines ${ldd_out})
	    foreach (line ${ldd_out_lines})
		string (REGEX REPLACE " \(.*\)" "" pruned ${line})
		string (STRIP ${pruned} dep_filename)
		if (IS_ABSOLUTE ${dep_filename})
		    is_macos_system_lib (${dep_filename} sys_lib)
		    if (sys_lib EQUAL 0 OR INSTALL_SYSLIBS STREQUAL "true")
                        string( REGEX REPLACE ".framework/.*$" ".framework" framework ${dep_filename} )
			message( STATUS "${framework} must be installed" )
                        set( DEPENDENCIES ${framework} ${DEPENDENCIES} PARENT_SCOPE)
			#install_library_with_deps( ${dep_filename} )
		    endif()
		endif()
	    endforeach()
	else()
	    message( WARNING "Executable ${exe} does not exist!" )
	endif()
    endforeach()
endfunction()




#
# Macro used to turn a list of .cpp/.h files into an absolute path for
# fluid files, and shortened relative paths for others.
#
# @bug: We need to do this on Windows, as xgettext chokes on too many
#       long paths.
#
macro( files_to_absolute_paths )
    set( PO_FILES )
    set( _no_short_name_regex "\\.cxx" ) # .cxx are fluid generated files
    set( PO_ABS_FILES  )
    foreach( filename ${SOURCES} ${HEADERS} )
	file(REAL_PATH ${filename} _abs_file )
	set( _short_name ${_abs_file} )
	set( _matched )
	foreach( match ${_no_short_name_regex} )
	    string( REGEX MATCH ${match} _matched ${_abs_file} )
	endforeach()
	if ( NOT _matched )
	    string( REGEX REPLACE ".*/lib/" "" _short_name ${_abs_file} )
	endif()
	set( PO_FILES ${_short_name} ${PO_FILES} )
        set( PO_SOURCES ${PO_FILES} ${PO_SOURCES} PARENT_SCOPE)
        set( PO_ABS_FILES ${_abs_file} ${PO_ABS_FILES}  )
        set( PO_ABS_SOURCES ${PO_ABS_FILES} ${PO_ABS_SOURCES} PARENT_SCOPE)
    endforeach()
endmacro()

#
# This function is to be used to link against libstdc++fs on 
#
function( set_required_build_settings_for_GCC8 )
    # Always link with libstdc++fs.a when using GCC 8.
    # Note: This command makes sure that this option comes pretty late on the cmdline.
    link_libraries( "$<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:-lstdc++fs>" )
endfunction()


