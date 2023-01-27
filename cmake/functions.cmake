# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

function (FLTK_RUN_FLUID TARGET SOURCES)
    set (CXX_FILES)
    foreach (src ${SOURCES})
	if ("${src}" MATCHES "\\.fl$")
	    string(REGEX REPLACE "(.*/)?(.*).fl" \\2 basename ${src})
	    add_custom_command(
		OUTPUT "${basename}.cxx" "${basename}.h"
		COMMAND ${FLTK_FLUID_EXECUTABLE} -c ${CMAKE_CURRENT_SOURCE_DIR}/${src}
		DEPENDS ${src}
		MAIN_DEPENDENCY ${src}
		)
	    list (APPEND CXX_FILES "${CMAKE_CURRENT_BINARY_DIR}/${basename}.cxx")
	endif ("${src}" MATCHES "\\.fl$")
    endforeach ()
    set (${TARGET} ${CXX_FILES} PARENT_SCOPE)
endfunction()

#
# Function used to discard system DSOS
#
function( is_system_lib TARGET ISSYSLIB )
    set( _syslibs libOpenGL libGL libGLdispatch libGLX libX nvidia libdrm2 libpthread libresolv libm librt libdl libxcb libasound libgpg-error libfontconfig libfreetype libxshmfence libc libstdc libgcc_s ld-linux )
    set( ${ISSYSLIB} 0 PARENT_SCOPE)
    foreach( lib ${_syslibs} )
	if ("${TARGET}" MATCHES "${lib}")
	    set( ${ISSYSLIB} 1 PARENT_SCOPE)
	    message( STATUS "   IS SYSLIB" )
	endif()
	if ("${TARGET}" MATCHES "${CMAKE_INSTALL_PREFIX}/lib")
	    set( ${ISSYSLIB} 1 PARENT_SCOPE)
	    message( STATUS "   IS INSTALLED" )
	endif()
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
	    execute_process(COMMAND ldd ${exe} OUTPUT_VARIABLE ldd_out)
	    string (REPLACE "\n" ";" ldd_out_lines ${ldd_out})
	    foreach (line ${ldd_out_lines})
		string (REGEX REPLACE "^.* => | \(.*\)" "" pruned ${line})
		string (STRIP ${pruned} dep_filename)
		if (IS_ABSOLUTE ${dep_filename})
		    message( STATUS "PARSING ${dep_filename}" )
		    is_system_lib (${dep_filename} sys_lib)
		    if (sys_lib EQUAL 0 OR INSTALL_SYSLIBS STREQUAL "true")
			list (FIND dependencies ${dep_filename} found)
			if (found LESS 0)
			    list (APPEND dependencies ${dep_filename})
			endif()
		    endif()
		endif()
	    endforeach()
	endif()
    endforeach()
    set(${DEPENDENCIES} ${dependencies} PARENT_SCOPE)
endfunction()
