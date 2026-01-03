#
# Pre-pare mrv2.app
#
function(install_mrv2_bin_glob _binglob)
    file(GLOB _bins "${_binglob}")
    foreach( _bin ${_bins} )
	file(COPY ${_bin}
	    DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources/bin)
    endforeach()
endfunction()

function(install_mrv2_lib_glob _libglob)
    file(GLOB _libs "${_libglob}")
    foreach( _lib ${_libs} )
	message(STATUS "Copying ${_lib} to ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources/lib")
	file(COPY ${_lib}
	    DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources/lib)
	install_macos_target_with_deps( ${_lib} )
    endforeach()
endfunction()

function(copy_mo_files SRC TARGET)
    file(GLOB _langs "${CPACK_PREPACKAGE}/share/locale/*")
    foreach( _fulldir ${_langs} )
	string(REGEX REPLACE ".*/locale/" "" _lang ${_fulldir})
	set(_langdir share/locale/${_lang}/LC_MESSAGES)
	set(src_mo_file "${CPACK_PREPACKAGE}/${_langdir}/${SRC}-v${mrv2_VERSION}.mo")
	set(dst_mo_dir "${CPACK_PREPACKAGE}/${TARGET}.app/Contents/Resources/${_langdir}/")
	file(MAKE_DIRECTORY ${dst_mo_dir})
	if (NOT EXISTS "${src_mo_file}")
	    message(STATUS "FAILED: ${src_mo_file} does not exist!")
	    message(FATAL_ERROR ".mo generations must have failed.")
	else()
	    message(STATUS "Copying ${src_mo_file} to ${dst_mo_dir}")
	    file(COPY ${src_mo_file} DESTINATION ${dst_mo_dir} )
	endif()
    endforeach()
endfunction()


function(install_vulkan_lib_glob _libglob APPNAME)
    if (DEFINED VULKAN_SDK AND EXISTS ${VULKAN_SDK})
	set(_vulkan_found FALSE)
	file(GLOB _libs "${VULKAN_SDK}/lib/${_libglob}.dylib")
	foreach( _lib ${_libs} )
	    file(COPY ${_lib}
		DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/lib
		FOLLOW_SYMLINK_CHAIN)
	    set(_vulkan_found TRUE)
	endforeach()
	if(NOT _vulkan_found)
	    message(FATAL_ERROR "FAILED: VULKAN_SDK set to ${VULKAN_SDK} but ${_libglob} not found")
	endif()
	return()
    endif()
    set(_vulkan_found FALSE)
    file(GLOB _libs "/opt/homebrew/lib/${_libglob}.dylib")
    foreach( _lib ${_libs} )
	file(COPY ${_lib}
	    DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/lib
	    FOLLOW_SYMLINK_CHAIN)
	set(_vulkan_found TRUE)
    endforeach()
    if (NOT _vulkan_found)
	file(GLOB _libs "/usr/local/lib/${_libglob}.dylib")
	foreach( _lib ${_libs} )
	    file(COPY ${_lib}
		DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/lib
		FOLLOW_SYMLINK_CHAIN)
	endforeach()
    endif()
endfunction()

function(install_vulkan_icd_filenames APPNAME)
    #
    # Try Vulkan SDK first
    #
    set(_vulkan_found FALSE)
    if (DEFINED VULKAN_SDK AND EXISTS ${VULKAN_SDK})
	file(GLOB _libs
	    "${VULKAN_SDK}/etc/vulkan*"
	    "${VULKAN_SDK}/../etc/vulkan*")
	foreach( _lib ${_libs} )
	    message(STATUS "Copying ${_lib} to ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/etc")
	    file(COPY ${_lib}
		DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/etc
		FOLLOW_SYMLINK_CHAIN)
	    set(_vulkan_found TRUE)
	endforeach()
	if(NOT _vulkan_found)
	    message(FATAL_ERROR "FAILED: VULKAN_SDK set to ${VULKAN_SDK} but ${VULKAN_SDK}/etc/vulkan/icd.d/*.json not found")
	endif()
	return()
    endif()

    #
    # Try /usr/local next
    #
    
    file(GLOB _libs
	"/opt/homebrew/etc/vulkan*")
    foreach( _lib ${_libs} )
	message(STATUS "Copying ${_lib} to ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/etc/")
	file(COPY ${_lib}
	    DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/etc/
	    FOLLOW_SYMLINK_CHAIN)
	set(_vulkan_found TRUE)
    endforeach()
    if(_vulkan_found)
	return()
    endif()
    
    file(GLOB _libs
	"/usr/local/etc/vulkan*")
    foreach( _lib ${_libs} )
	message(STATUS "Copying ${_lib} to ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/etc")
	file(COPY ${_lib}
	    DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/etc/
	    FOLLOW_SYMLINK_CHAIN)
	set(_vulkan_found TRUE)
    endforeach()
    if(_vulkan_found)
	return()
    endif()

    
    #
    # Try user's home dir next
    #
    set(HOME $ENV{HOME})
    file(GLOB _libs
	"${HOME}/etc/vulkan*"
	"${HOME}/share/vulkan*")
    foreach( _lib ${_libs} )
	message(STATUS "Copying ${_lib} to ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/etc")
	file(COPY ${_lib}
	    DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/etc/
	    FOLLOW_SYMLINK_CHAIN)
	set(_vulkan_found TRUE)
    endforeach()
    if(_vulkan_found)
	return()
    endif()

    #
    # Try System's location last
    #
    file(GLOB _libs
	"/etc/vulkan*")
    foreach( _lib ${_libs} )
	message(STATUS "Copying ${_lib} to ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/etc")
	file(COPY ${_lib}
	    DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/etc/
	    FOLLOW_SYMLINK_CHAIN)
	set(_vulkan_found TRUE)
    endforeach()
    
    if(NOT _vulkan_found)
	message(FATAL_ERROR "FAILED: Could not locate /etc/vulkan/icd.d/*.json")
    endif()
endfunction()

function(install_vulkan_layers APPNAME)
    #
    # Try Vulkan SDK first
    #
    set(_vulkan_found FALSE)
    if (DEFINED VULKAN_SDK AND EXISTS ${VULKAN_SDK})
	file(GLOB _dirs "${VULKAN_SDK}/share/vulkan*")
	foreach( _dir ${_dirs} )
	    message(STATUS "Copying ${_dir} to ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/share/")
	    file(COPY ${_dir}
		DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/share/
		FOLLOW_SYMLINK_CHAIN)
	    set(_vulkan_found TRUE)
	endforeach()
	if(_vulkan_found)
	    return()
	endif()
	if(NOT _vulkan_found)
	    message(FATAL_ERROR "FAILED: VULKAN_SDK set to ${VULKAN_SDK} but ${VULKAN_SDK}/share/vulkan* not found")
	endif()
	return()
    endif()

    #
    # Try /usr/local next
    #
    
    file(GLOB _dirs "/opt/homebrew/vulkan-validationlayers/share/vulkan*")
    foreach( _dir ${_dirs} )
	message(STATUS "Copying ${_dir} to ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/share/")
	file(COPY ${_dir}
	    DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/share/
	    FOLLOW_SYMLINK_CHAIN)
	set(_vulkan_found TRUE)
    endforeach()
    if(NOT _vulkan_found)
	message(STATUS "VULKAN_SDK set to ${VULKAN_SDK} but ${VULKAN_SDKw}/share/vulkan* not found")
    endif()

    file(GLOB _dirs "/usr/local/opt/vulkan-validationlayers/share/vulkan*")
    foreach( _dir ${_dirs} )
	message(STATUS "Copying ${_dir} to ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/share/")
	file(COPY ${_dir}
	    DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/share/
	    FOLLOW_SYMLINK_CHAIN)
	set(_vulkan_found TRUE)
    endforeach()
    if(_vulkan_found)
	return()
    endif()
    
    #
    # Try user's home dir next
    #
    set(HOME $ENV{HOME})
    file(GLOB _dirs "${HOME}/share/vulkan*")
    foreach( _dir ${_dirs} )
	message(STATUS "Copying ${_dir} to ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/share*")
	file(COPY ${_dir}
	    DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/share/vulkan
	    FOLLOW_SYMLINK_CHAIN)
	set(_vulkan_found TRUE)
    endforeach()
    if(_vulkan_found)
	return()
    endif()

    #
    # Try System's location last
    #
    file(GLOB _dirs "/share/vulkan*")
    foreach( _dir ${_dirs} )
	message(STATUS "Copying ${_dir} to ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/share/")
	file(COPY ${_dir}
	    DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/share/
	    FOLLOW_SYMLINK_CHAIN)
	set(_vulkan_found TRUE)
    endforeach()
    
    if(NOT _vulkan_found)
	message(FATAL_ERROR "FAILED: Could not locate share/vulkan/*")
    endif()
endfunction()

file(COPY ${CPACK_PREPACKAGE}/bin/mrv2.sh
    DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources/bin)
file(COPY ${CPACK_PREPACKAGE}/bin/mrv2
    DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources/bin)
file(COPY ${CPACK_PREPACKAGE}/bin/license_helper
    DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources/bin)
file(COPY ${CPACK_PREPACKAGE}/bin/environment.sh
    DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources/bin)
file(COPY ${CPACK_PREPACKAGE}/bin/install_dmg.sh
    DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources/bin)

if (EXISTS ${CPACK_PREPACKAGE}/bin/python.sh)
    install_mrv2_bin_glob("${CPACK_PREPACKAGE}/bin/python*")
    install_mrv2_bin_glob("${CPACK_PREPACKAGE}/bin/pip*")
endif()

file(COPY ${CPACK_PREPACKAGE}/certs
    DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources)
file(COPY ${CPACK_PREPACKAGE}/colors
    DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources)
if (EXISTS ${CPACK_PREPACKAGE}/docs)
    file(COPY ${CPACK_PREPACKAGE}/docs
	DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources)
endif()
file(COPY ${CPACK_PREPACKAGE}/lib
    DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources)
if (EXISTS ${CPACK_PREPACKAGE}/libraries)
    file(COPY ${CPACK_PREPACKAGE}/libraries
	DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources)
endif()
file(COPY ${CPACK_PREPACKAGE}/ocio
    DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources)
if (EXISTS ${CPACK_PREPACKAGE}/plugin)
    file(COPY ${CPACK_PREPACKAGE}/plugin
	DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources)
endif()
file(COPY ${CPACK_PREPACKAGE}/presets
    DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources)
file(COPY ${CPACK_PREPACKAGE}/python
    DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/Resources)

#
# Install .mo translation files
#
copy_mo_files(mrv2 ${mrv2_NAME})

set(VULKAN_SDK $ENV{VULKAN_SDK})
message(STATUS "VULKAN_SDK set to ${VULKAN_SDK}")

#
# Copy vmrv2/mrv2 launcher with the name of the directory.
#
file(COPY ${CPACK_PREPACKAGE}/bin/launcher
    DESTINATION ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/MacOS/)
file(RENAME ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/MacOS/launcher
    ${CPACK_PREPACKAGE}/${mrv2_NAME}.app/Contents/MacOS/${mrv2_NAME})

    
#
install_mrv2_lib_glob("${CPACK_PREPACKAGE}/lib/libfltk*")
install_mrv2_lib_glob("${CPACK_PREPACKAGE}/lib/libintl*")
install_mrv2_lib_glob("${CPACK_PREPACKAGE}/lib/libomp*")
install_mrv2_lib_glob("${CPACK_PREPACKAGE}/lib/libplacebo*")
install_mrv2_lib_glob("${CPACK_PREPACKAGE}/lib/libz*" )
    
if (MRV2_BACKEND STREQUAL "VK")
    install_vulkan_lib_glob("libglslang*" vmrv2)
    install_vulkan_lib_glob("libMoltenVK*" vmrv2)
    install_vulkan_lib_glob("libvulkan*" vmrv2)
    install_vulkan_icd_filenames(vmrv2)
endif()

#
# Pre-pare hdr.app if present
#
if (EXISTS ${CPACK_PREPACKAGE}/hdr.app)
    #
    # Function to install a glob of libraries in hdr.app
    #
    function(install_hdr_lib_glob _libglob)
	file(GLOB _libs "${_libglob}")
	foreach( _lib ${_libs} )
	    file(COPY ${_lib}
		DESTINATION ${CPACK_PREPACKAGE}/hdr.app/Contents/Resources/lib)
	    install_macos_target_with_deps( ${_lib} )
	endforeach()
    endfunction()
    
    file(COPY ${CPACK_PREPACKAGE}/bin/hdr.sh
	DESTINATION ${CPACK_PREPACKAGE}/hdr.app/Contents/Resources/bin)
    file(COPY ${CPACK_PREPACKAGE}/bin/hdr
	DESTINATION ${CPACK_PREPACKAGE}/hdr.app/Contents/Resources/bin)
    file(COPY ${CPACK_PREPACKAGE}/bin/environment.sh
	DESTINATION ${CPACK_PREPACKAGE}/hdr.app/Contents/Resources/bin)


    #
    # Common libraries
    #
    install_hdr_lib_glob("${CPACK_PREPACKAGE}/lib/libfltk*")
    install_hdr_lib_glob("${CPACK_PREPACKAGE}/lib/libglslang*")
    install_hdr_lib_glob("${CPACK_PREPACKAGE}/lib/libintl*")
    install_hdr_lib_glob("${CPACK_PREPACKAGE}/lib/libndi*")
    install_hdr_lib_glob("${CPACK_PREPACKAGE}/lib/libplacebo*")
    install_hdr_lib_glob("${CPACK_PREPACKAGE}/lib/libz*")
    install_hdr_lib_glob("${CPACK_PREPACKAGE}/lib/libMoltenVK*")

    # Vulkan libraries For Apple Silicon or Intel machines
    install_vulkan_lib_glob("libvulkan*" hdr)
    
    install_vulkan_icd_filenames(hdr)
    # install_vulkan_layers(hdr)
    
    #
    # Install .mo translation files
    #
    copy_mo_files(hdr hdr)
endif()

#
# Clean up main staging area
#
file(REMOVE_RECURSE ${CPACK_PREPACKAGE}/bin)
file(REMOVE_RECURSE ${CPACK_PREPACKAGE}/certs)
file(REMOVE_RECURSE ${CPACK_PREPACKAGE}/colors)
file(REMOVE_RECURSE ${CPACK_PREPACKAGE}/docs)
file(REMOVE_RECURSE ${CPACK_PREPACKAGE}/lib)
if (EXISTS ${CPACK_PREPACKAGE}/libraries)
    file(REMOVE_RECURSE ${CPACK_PREPACKAGE}/libraries)
endif()
file(REMOVE_RECURSE ${CPACK_PREPACKAGE}/ocio)
if (EXISTS ${CPACK_PREPACKAGE}/plugin)
    file(REMOVE_RECURSE ${CPACK_PREPACKAGE}/plugin)
endif()
file(REMOVE_RECURSE ${CPACK_PREPACKAGE}/presets)
file(REMOVE_RECURSE ${CPACK_PREPACKAGE}/python)
file(REMOVE_RECURSE ${CPACK_PREPACKAGE}/share)
