# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Function used to take .fl files (GUI creator into .cxx / .h files.
#
# Call like FLTK_RUN_FLUID( output_var ".fl SOURCES" )
#
#
function (FLTK_RUN_FLUID TARGET SOURCES)
    set (CXX_FILES)
    foreach (src ${SOURCES})
	if ("${src}" MATCHES "\\.fl$")
	    string(REGEX REPLACE "(.*/)?(.*).fl" \\2 basename ${src})
	    add_custom_command(
		OUTPUT "${basename}.cxx" "${basename}.h"
		COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && ${FLTK_FLUID_EXECUTABLE} -c "${CMAKE_CURRENT_SOURCE_DIR}/${src}"
		DEPENDS ${src}
		MAIN_DEPENDENCY ${src}
		)
	    set( _cxx_file "${CMAKE_CURRENT_BINARY_DIR}/${basename}.cxx" )
	    list (APPEND CXX_FILES ${_cxx_file} )
	endif ("${src}" MATCHES "\\.fl$")
    endforeach ()
    set (${TARGET} ${CXX_FILES} PARENT_SCOPE)
endfunction()


#
# Function used to discard system DSOS or those already installed
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
    # Vulkan Libs to distribute on Linux and macOS
    # 
    set(_vulkan_libs
	libglslang
	libshaderc_combined
	libSPIRV-Tools
	libSPIRV-Tools-opt
    )

    if (APPLE)
	# On Apple we must distribute libvulkan and libMoltenVK
	list(APPEND _vulkan_libs
	    libMoltenVK
	    libvulkan)
    endif()
    
    #
    # List of libraries that are accepted to distribute
    #
    set( _acceptedlibs
	libavcodec
	libavdevice
	libavfilter
	libavformat
	libavutil
	libcairo
	libcrypto
	liblcms2
	libMaterial
	libmd
	libndi
	libosd
	libraw
	libraw_r
	libtbb
	libusd 
	${_vulkan_libs})

    #
    # List of system kde libraries that should not be distributed
    #
    set(_kde_libs
	libkwin
	
	libKF5ConfigCore
	libKF5ConfigGui
	libKF5CoreAddons
	libKF5KIO
	libKF5Notifications
	libKF5Plasma
	libKF5WaylandClient
	libKF5WindowSystem
	
	libKF6ConfigCore
	libKF6ConfigGui
	libKF6CoreAddons
	libKF6KIO
	libKF6Notifications
	libKF6Plasma
	libKF6WaylandClient
	libKF6WindowSystem
	
	libinput
    )

    set(_qt_libs	
	libQt5Core
	libQt5DBus
	libQt5Gui
	libQt5Widgets
	libQt5WaylandClient
	libQt5WaylandCompositor

	libQt6Core
	libQt6DBus
	libQt6Gui
	libQt6Widgets
	libQt6WaylandClient
	libQt6WaylandCompositor
    )
    
    set(_gnome_libs
	libcairo
	libdrm
	libdrm2
	libgio
	libglib
	libgobject
	libpango
	libwayland-client
	libwayland-cursor
	libwayland-egl
	libwayland-server
	libxkbcommon
    )

    set(_x11_libs
	libX11
	libX11-xcb
	libXau
	libXaw7
	libXaw
	libXcomposite
	libXcursor
	libXdamage
	libXdmcp
	libXext
	libXfixes
	libXinerama
	libXi
	libXmu
	libXmuu
	libXpm
	libXrender
	libXrandr
	libXRes
	# libXss   # not present in some distros like Rocky 10
	libxshmfence
	libXt
	libXtst
	libXvMC
	libXvMCW
	libXv
	libXxf86dga
	libXxf86vm
	
	libxcb-shape
	libxcb-xfixes
	libxcb-render
	libxcb-randr
	libxcb-shm
	libxcb-composite
	libxcb
    )

    set(_opengl_libs
	libEGL
	libGL
	libGLdispatch
	libGLX
	libOpenGL
	nvidia
    )

    set(_audio_libs
	libasound
	libpulse
	libpulse-simple
	librtaudio
    )

    set(_macos_libs )
    if (APPLE)
	set(_macos_libs
	    X11
	    tcl
	    tk
	)
    endif()
    
    #
    # List of system libraries that should *NOT* be distributed
    #
    set(_syslibs
	linux-vdso
	ld-linux
	libblkid
	libc
	libcap         # was accepted
	libdbus        # was accepted before
	libdl
	libharfbuzz
	libfontconfig
	libfreetype
	libgbm
	libgcc_s
	libgpg-error
	libm
	libmount        # was accepted before - broke in Fedora 42, Wayland lib
	libpng
	libpthread
	libresolv
	librt
	libsystemd
	libtinfo
	libudev
	libutil          # was accepted before
	libstdc
	libvulkan        # On Linux, we don't distribute libvulkan
	libz
	vulkan-          # On Windows, we don't distribute vulkan-1.dll
	${_audio_libs}
	${_kde_libs}
	${_gnome_libs}
	${_qt_libs}
	${_x11_libs}
	${_opengl_libs}
	${_macos_libs}
    )

    
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

set(INSTALLED_LIBRARIES "" CACHE INTERNAL "List of installed libraries")

#
# Function used to install a library  with all .so dependencies
#
function(install_library_with_links LIBRARY)
    list(FIND INSTALLED_LIBRARIES "${LIBRARY}" already_installed)
    if (already_installed GREATER -1)
        message(STATUS "SKIPPED (already installed) ${LIBRARY}")
        return()
    endif()
    
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

    list(APPEND INSTALLED_LIBRARIES "${LIBRARY}")
    set(INSTALLED_LIBRARIES "${INSTALLED_LIBRARIES}" CACHE INTERNAL "List of installed libraries")

endfunction()

#
# Function used to get runtime dependencies as cmake GET_RUNTIME_DEPENDENCIES is
# broken.
#

set(PROCESSED_LIBRARIES "" CACHE INTERNAL "List of processed libraries")

function( get_runtime_dependencies TARGET )

    foreach (exe ${TARGET})
	if ( EXISTS ${exe} )
	    message( STATUS "PARSING ${exe} for DSOs...." )
	    execute_process(COMMAND ldd ${exe} OUTPUT_VARIABLE ldd_out)
	    string (REPLACE "\n" ";" ldd_out_lines ${ldd_out})
	    foreach (line ${ldd_out_lines})
		string (REGEX REPLACE "^.* => | \(.*\)" "" pruned ${line})
		string (STRIP ${pruned} dep_filename)
		if (IS_ABSOLUTE ${dep_filename})

		    list(FIND PROCESSED_LIBRARIES "${dep_filename}" already_processed)
                    if (already_processed EQUAL -1)
                        list(APPEND PROCESSED_LIBRARIES "${dep_filename}")
                        set(PROCESSED_LIBRARIES "${PROCESSED_LIBRARIES}" CACHE INTERNAL "List of processed libraries")

			is_system_lib (${dep_filename} sys_lib)
			if (sys_lib EQUAL 0 OR INSTALL_SYSLIBS STREQUAL "true")
			    install_library_with_links( ${dep_filename} )
			endif()
                    endif()
		else()
		    is_system_lib (${dep_filename} sys_lib)
		    if (sys_lib EQUAL 0)
			message("${dep_filename} is NOT absolute - ${line}")
		    endif()
		endif()
	    endforeach()
	else()
	    message( WARNING "Executable or library ${exe} does not exist!" )
	endif()
    endforeach()

endfunction()

function( install_macos_target_with_deps TARGET )
    foreach (_target ${TARGET})
	if ( EXISTS "${_target}" )
	    execute_process(COMMAND otool -L "${_target}" OUTPUT_VARIABLE ldd_out)
	    string (REPLACE "\n" ";" ldd_out_lines ${ldd_out})
	    foreach (line ${ldd_out_lines})
		string(REGEX REPLACE " \(.*\)" "" pruned ${line})
		string(REGEX REPLACE ":$" "" pruned ${pruned})
		string(STRIP ${pruned} dep_filename)
		if ("${dep_filename}" STREQUAL "${_target}")
		    continue()
		endif()
		if (IS_ABSOLUTE "${dep_filename}")
		    list(FIND PROCESSED_LIBRARIES "${dep_filename}" already_processed)
                    if (already_processed EQUAL -1)
                        list(APPEND PROCESSED_LIBRARIES "${dep_filename}")
                        set(PROCESSED_LIBRARIES "${PROCESSED_LIBRARIES}" CACHE INTERNAL "List of processed libraries")
			
			is_macos_system_lib ("${dep_filename}" sys_lib)
			if (sys_lib EQUAL 0 OR INSTALL_SYSLIBS STREQUAL "true")
                            string( REGEX REPLACE ".framework/.*$" ".framework" framework "${dep_filename}" )
			    install_library_with_links( "${framework}" )
			    if ("${framework}" STREQUAL "${dep_filename}")
				install_macos_target_with_deps( "${framework}" )
			    endif()
			endif()
		    endif()
		endif()
	    endforeach()
	else()
	    message( WARNING "TARGET ${_target} does not exist!" )
	endif()
    endforeach()
endfunction()

#
# Function used to get macos runtime dependencies as cmake
# GET_RUNTIME_DEPENDENCIES is broken.
#
function( get_macos_runtime_dependencies TARGET )
    install_macos_target_with_deps( "${TARGET}" )
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
    set( _exclude_regex "\\.mm" ) # macOS .mm files are not considered
    set( _no_short_name_regex "\\.cxx" ) # .cxx are fluid generated files
    set( PO_ABS_FILES  )
    foreach( filename ${SOURCES} ${HEADERS} )
	file(REAL_PATH ${filename} _abs_file )
	set( _matched )
	string( REGEX MATCH ${_exclude_regex} _matched ${_abs_file} )
	if ( _matched )
            continue()
	endif()
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
# Macro used to turn a list of .cpp/.h files into an absolute path for
# fluid files, and shortened relative paths for others.
#
# @bug: We need to do this on Windows, as xgettext chokes on too many
#       long paths.
#
macro( hdr_files_to_absolute_paths )
    set( PO_HDR_FILES )
    set( _exclude_regex "\\.mm" ) # macOS .mm files are not considered
    set( _no_short_name_regex "\\.cxx" ) # .cxx are fluid generated files
    set( PO_HDR_ABS_FILES  )
    foreach( filename ${SOURCES} ${HEADERS} )
	file(REAL_PATH ${filename} _abs_file )
	set( _matched )
	string( REGEX MATCH ${_exclude_regex} _matched ${_abs_file} )
	if ( _matched )
            continue()
	endif()
	set( _short_name ${_abs_file} )
	set( _matched )
	foreach( match ${_no_short_name_regex} )
	    string( REGEX MATCH ${match} _matched ${_abs_file} )
	endforeach()
	if ( NOT _matched )
	    string( REGEX REPLACE ".*/lib/" "" _short_name ${_abs_file} )
	endif()
	set( PO_HDR_FILES ${_short_name} ${PO_HDR_FILES} )
        set( PO_HDR_SOURCES ${PO_HDR_FILES} ${PO_HDR_SOURCES} PARENT_SCOPE)
        set( PO_HDR_ABS_FILES ${_abs_file} ${PO_HDR_ABS_FILES}  )
        set( PO_HDR_ABS_SOURCES
	    ${PO_HDR_ABS_FILES}
	    ${PO_HDR_ABS_SOURCES} PARENT_SCOPE)
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


function(copy_pdbs_to_install PDB_FILE INSTALL_DIR)
    if (${PDB_FILE} MATCHES ".*_CPack_Packages.*")
	message(STATUS "SKIPPING CPack_Package ${PDB_FILE}")
	return()
    endif()
    if (${PDB_FILE} MATCHES ".*fluid.*.pdb")
	message(STATUS "SKIPPING ${PDB_FILE}")
	return()
    endif()
    if (${PDB_FILE} MATCHES "test_.*")
	message(STATUS "SKIPPING ${PDB_FILE}")
	return()
    endif()
    if(NOT EXISTS ${PDB_FILE})
	message(FATAL_ERROR ".pdb file ${PDB_FILE} does not exist")
    endif()
    # Copy the PDB file to the install directory
    file(INSTALL ${PDB_FILE}
	DESTINATION ${INSTALL_DIR}/debug)
endfunction()

function(copy_pdbs DIR INSTALL_DIR)
    file(GLOB _pdbs "${DIR}/*")
    foreach(_pdb ${_pdbs})
	if(IS_DIRECTORY ${_pdb})
	    copy_pdbs(${_pdb} ${INSTALL_DIR})
	else()
	    if (${_pdb} MATCHES ".*\\.pdb$")
		file(REAL_PATH ${_pdb} _pdb )
		copy_pdbs_to_install(${_pdb} ${INSTALL_DIR})
	    endif()
	endif()
    endforeach()
endfunction()
