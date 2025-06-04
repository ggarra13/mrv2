# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

cmake_policy(SET CMP0009 NEW)

message( STATUS "----------------------------------------prepackage------------------------------------" )
message( STATUS "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}" )
message( STATUS "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}" )
message( STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}" )

#
# Find mrv2's root dir by brute force, looking for directory that has
# our "cmake/version.cmake" file.
#
set( _current_dir ${CMAKE_CURRENT_BINARY_DIR})
set( _found_root_dir FALSE)
while( NOT _found_root_dir )
    set(_current_dir "${_current_dir}/..")
    file(REAL_PATH ${_current_dir} _current_dir )
    set(_mrv2_version "${_current_dir}/cmake/version.cmake" )
    if (EXISTS ${_mrv2_version})
	set(ROOT_DIR ${_current_dir})
	set(_found_root_dir TRUE)
    endif()
endwhile()

file(REAL_PATH ${ROOT_DIR} ROOT_DIR )
message( STATUS "mrv2 ROOT_DIR=${ROOT_DIR}" )


include( "${ROOT_DIR}/cmake/version.cmake" )
include( "${ROOT_DIR}/cmake/functions.cmake" )


# Function to remove __pycache__ directories
function(remove_pycache_directories dir)
    file(GLOB_RECURSE pycache_dirs RELATIVE ${dir} "__pycache__")

    foreach(pycache_dir IN LISTS pycache_dirs)
        file(REMOVE_RECURSE "${dir}/${pycache_dir}")
    endforeach()
endfunction()

# Function to remove python directories and modules
function(remove_python_directories _dirs)
    if ( NOT "${_dirs}" STREQUAL "" )
	foreach( _dir ${_dirs} )
	    remove_pycache_directories(${_dir})
	    message(STATUS "Removing ${_dir}")
	endforeach()
	
	file( REMOVE_RECURSE ${_dirs} )
    endif()
endfunction()

if( UNIX AND NOT APPLE )
    #
    # \@bug: Linux CMAKE_INSTALL_PREFIX is broken and not pointing to
    #        pre-packaging directory!!!
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
# Remove usd directory from lib/ directory on Windows
#
if(WIN32)
    message( STATUS "Removing ${CPACK_PREPACKAGE}/lib/usd")
    file( REMOVE_RECURSE "${CPACK_PREPACKAGE}/lib/usd" )
endif()

#
# Remove .a, .lib and .dll files from packaging lib/ directory
#
message( STATUS "Removing static and DLLs from ${CPACK_PREPACKAGE}/lib")
file( GLOB STATIC_LIBS "${CPACK_PREPACKAGE}/lib/*.a"
		       "${CPACK_PREPACKAGE}/lib/*.lib"
		       "${CPACK_PREPACKAGE}/lib/*.dll" )
		   

if ( NOT "${STATIC_LIBS}" STREQUAL "" )
    file( REMOVE ${STATIC_LIBS} )
endif()


#
# Remove include files from packaging directory
#
message( STATUS "Removing ${CPACK_PREPACKAGE}/include" )
file( REMOVE_RECURSE "${CPACK_PREPACKAGE}/include" )

#
# List of python module globs to remove if present
# 
set( _pythonBuiltinModules 
    test*
    config*
    ctypes/test*
    distutils/test*
    idlelib/idle_test*
    lib2to3/test*
    sqlite3/test*
    tkinter/test*
    unittest*
)

set( _pythonSiteModules 
    alabaster*
    babel*
    click*
    colorama*
    docutils*
    filelock*
    fltk14/*.cpp
    fltk14/*.h
    fsspec*
    huggingface-hub*
    imagesize*
    Jinja*
    joblib*
    libretranslatepy*
    lxml*
    MarkupSafe*
    meson*
    mpmath*
    numpy*
    networkx*
    pillow*
    PIL*
    polib*
    pyyaml*
    pyYAML*
    pygments*
    Pygments*
    pytz*
    regex*
    sacremoses*
    safetensors*
    sentencepiece*
    snowballstemmer*
    sphinx*
    sympy*
    tokenizers*
    torch*
    transformers*
    translate*
    typing-extensions*
    tqdm*
    unittest*
)

#
# Install system .SO dependencies
#
if(UNIX)

    #
    # Under Rocky Linux, DSOs sometimes go to lib64/.
    # Copy them to lib/ directory
    #
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
	file(REMOVE_RECURSE ${linux_lib64_dir})
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
    set(MRV2_PYTHON_LIBDIR "")
    if (MRV2_PYTHON_LIB_DIRS STREQUAL "")
	message(WARNING "Could not locate any python version in:\n${CPACK_PREPACKAGE}/lib/python*")
    else()
	list(GET MRV2_PYTHON_LIB_DIRS -1 MRV2_PYTHON_LIB_DIR)
    endif()
    
    set(MRV2_PYTHON_SITE_PACKAGES_DIR "${MRV2_PYTHON_LIB_DIR}/site-packages")

    set( MRV2_EXES "${CPACK_PREPACKAGE}/bin/mrv2" )

    if (EXISTS "${CPACK_PREPACKAGE}/bin/hdr")
	list(APPEND MRV2_EXES "${CPACK_PREPACKAGE}/bin/hdr" )
    endif()
    
    #
    # We need to get the dependencies of the python DSOs to avoid
    # issues like openssl and libcrypto changing between Rocky Linux
    # and Ubuntu 22.04.5 or macOS's libb2.
    #
    set(MRV2_PYTHON_DSO_DIR "${MRV2_PYTHON_LIB_DIR}/lib-dynload")
    file(GLOB python_dsos "${MRV2_PYTHON_DSO_DIR}/*.so")
    list(APPEND MRV2_EXES ${python_dsos} )
	
    if ( APPLE )
	#
	# Get DYLIB dependencies of components
	#
	get_macos_runtime_dependencies( "${MRV2_EXES}" )
	
    else()
	#
	# Get DSO dependencies of components
	#
	get_runtime_dependencies( "${MRV2_EXES}" )
    endif()
elseif(WIN32)
    #
    # Set python's site-packages dir for .exe installer.
    #
    # When building an .exe installer on Windows, the site-packages will
    # be inside an applications component directory.
    #
    set(MRV2_APP_DIR "${CPACK_PREPACKAGE}/applications")

    if (EXISTS "${MRV2_APP_DIR}/bin/mrv2.exe")
	file(CREATE_LINK
	    "${MRV2_APP_DIR}/bin/mrv2.exe"
	    "${MRV2_APP_DIR}/bin/mrv2-v${mrv2_VERSION}.exe") 
    endif()
    
    set(MRV2_PYTHON_APP_LIB_DIR "${MRV2_APP_DIR}/bin/Lib")
    set(MRV2_PYTHON_SITE_PACKAGES_DIR
	"${MRV2_PYTHON_APP_LIB_DIR}/site-packages")
    
    #
    # Don't pack sphinx and other auxiliary documentation libs in .exe
    #

    set(MRV2_PYTHON_GLOBS 
	"${MRV2_APP_DIR}/lib/Lib/")
    foreach( _dir ${_pythonBuiltinModules} )
	set(MRV2_PYTHON_GLOBS
	    ${MRV2_PYTHON_GLOBS}
	    "${MRV2_PYTHON_APP_LIB_DIR}/${_dir}")
    endforeach()
    
    foreach( _dir ${_pythonSiteModules} )
	set(MRV2_PYTHON_GLOBS
	    ${MRV2_PYTHON_GLOBS}
	    "${MRV2_PYTHON_SITE_PACKAGES_DIR}/${_dir}")
    endforeach()
    
    file(GLOB MRV2_UNUSED_PYTHON_DIRS ${MRV2_PYTHON_GLOBS})
    remove_python_directories( "${MRV2_UNUSED_PYTHON_DIRS}" )
    
    #
    # Set python's site-packages and lib dir for .zip.
    #
    # When building an .exe on Windows, the site-packages will
    # be inside an application directory.
    #
    set(MRV2_PYTHON_LIB_DIR "${CPACK_PREPACKAGE}/bin/Lib")
    set(MRV2_PYTHON_SITE_PACKAGES_DIR
	"${CPACK_PREPACKAGE}/bin/Lib/site-packages")
endif()


#
# Don't pack sphinx and other auxiliary documentation libs nor the tests
# for the libraries.
#
set(MRV2_PYTHON_GLOBS )

foreach( _dir ${_pythonBuiltinModules} )
    set(MRV2_PYTHON_GLOBS
	${MRV2_PYTHON_GLOBS}
	"${MRV2_PYTHON_LIB_DIR}/${_dir}")
endforeach()
    
foreach( _dir ${_pythonSiteModules} )
    set(MRV2_PYTHON_GLOBS
	${MRV2_PYTHON_GLOBS}
	"${MRV2_PYTHON_SITE_PACKAGES_DIR}/${_dir}")
endforeach()

file(GLOB MRV2_UNUSED_PYTHON_DIRS ${MRV2_PYTHON_GLOBS})
remove_python_directories( "${MRV2_UNUSED_PYTHON_DIRS}" )


if (APPLE)
    #
    # Pre-pare mrv2.app
    #
    function(install_mrv2_bin_glob _binglob)
	file(GLOB _bins "${_binglob}")
	foreach( _bin ${_bins} )
	    file(COPY ${_bin}
		DESTINATION ${CPACK_PREPACKAGE}/mrv2.app/Contents/Resources/bin)
	endforeach()
    endfunction()
    
    function(install_mrv2_lib_glob _libglob)
	file(GLOB _libs "${_libglob}")
	foreach( _lib ${_libs} )
	    file(COPY ${_lib}
		DESTINATION ${CPACK_PREPACKAGE}/mrv2.app/Contents/Resources/lib)
	endforeach()
    endfunction()
	
    function(copy_mo_files TARGET)
	file(GLOB _langs "${CPACK_PREPACKAGE}/share/locale/*")
	foreach( _fulldir ${_langs} )
	    string(REGEX REPLACE ".*/locale/" "" _lang ${_fulldir})
	    set(_langdir share/locale/${_lang}/LC_MESSAGES)
	    file(MAKE_DIRECTORY ${CPACK_PREPACKAGE}/${TARGET}.app/Contents/Resources/${_langdir}/)
	    file(COPY ${CPACK_PREPACKAGE}/${_langdir}/${TARGET}-v${mrv2_VERSION}.mo
		DESTINATION ${CPACK_PREPACKAGE}/${TARGET}.app/Contents/Resources/${_langdir})
	endforeach()
    endfunction()
    
    file(COPY ${CPACK_PREPACKAGE}/bin/mrv2.sh
	DESTINATION ${CPACK_PREPACKAGE}/mrv2.app/Contents/Resources/bin)
    file(COPY ${CPACK_PREPACKAGE}/bin/mrv2
	DESTINATION ${CPACK_PREPACKAGE}/mrv2.app/Contents/Resources/bin)
    file(COPY ${CPACK_PREPACKAGE}/bin/environment.sh
	DESTINATION ${CPACK_PREPACKAGE}/mrv2.app/Contents/Resources/bin)
    file(COPY ${CPACK_PREPACKAGE}/bin/install_dmg.sh
	DESTINATION ${CPACK_PREPACKAGE}/mrv2.app/Contents/Resources/bin)
    
    if (EXISTS ${CPACK_PREPACKAGE}/python.sh)
	install_mrv2_bin_glob("${CPACK_PREPACKAGE}/python*")
	install_mrv2_bin_glob("${CPACK_PREPACKAGE}/pip*")
    endif()
    
    install_mrv2_lib_glob("${CPACK_PREPACKAGE}/lib/libMoltenVK*")	
	
    file(COPY ${CPACK_PREPACKAGE}/colors
	DESTINATION ${CPACK_PREPACKAGE}/mrv2.app/Contents/Resources)
    if (EXISTS ${CPACK_PREPACKAGE}/docs)
	file(COPY ${CPACK_PREPACKAGE}/docs
	    DESTINATION ${CPACK_PREPACKAGE}/mrv2.app/Contents/Resources)
    endif()
    file(COPY ${CPACK_PREPACKAGE}/icons
	DESTINATION ${CPACK_PREPACKAGE}/mrv2.app/Contents/Resources)
    file(COPY ${CPACK_PREPACKAGE}/lib
	DESTINATION ${CPACK_PREPACKAGE}/mrv2.app/Contents/Resources)
    if (EXISTS ${CPACK_PREPACKAGE}/libraries)
	file(COPY ${CPACK_PREPACKAGE}/libraries
	    DESTINATION ${CPACK_PREPACKAGE}/mrv2.app/Contents/Resources)
    endif()
    file(COPY ${CPACK_PREPACKAGE}/ocio
	DESTINATION ${CPACK_PREPACKAGE}/mrv2.app/Contents/Resources)
    if (EXISTS ${CPACK_PREPACKAGE}/plugin)
	file(COPY ${CPACK_PREPACKAGE}/plugin
	    DESTINATION ${CPACK_PREPACKAGE}/mrv2.app/Contents/Resources)
    endif()
    file(COPY ${CPACK_PREPACKAGE}/presets
	DESTINATION ${CPACK_PREPACKAGE}/mrv2.app/Contents/Resources)
    file(COPY ${CPACK_PREPACKAGE}/python
	DESTINATION ${CPACK_PREPACKAGE}/mrv2.app/Contents/Resources)

    #
    # Install .mo translation files
    #
    copy_mo_files(mrv2)
    
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
		    message(FATAL_ERROR "VULKAN_SDK set to ${VULKAN_SDK} but ${_libglob} not found")
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
		file(GLOB _libs "${VULKAN_SDK}/etc/")
		foreach( _lib ${_libs} )
		    file(COPY ${_lib}
			DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/
			FOLLOW_SYMLINK_CHAIN)
		    set(_vulkan_found TRUE)
		endforeach()
		if(NOT _vulkan_found)
		    message(FATAL_ERROR "VULKAN_SDK set to ${VULKAN_SDK} but ${VULKAN_SDK}/etc/vulkan/icd.d/*.json not found")
		endif()
		return()
	    endif()

	    #
	    # Try /usr/local next
	    #
	    
	    file(GLOB _libs "/opt/homebrew/etc/vulkan")
	    foreach( _lib ${_libs} )
		file(COPY ${_lib}
		    DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/
		    FOLLOW_SYMLINK_CHAIN)
		set(_vulkan_found TRUE)
	    endforeach()
	    if(_vulkan_found)
		return()
	    endif()
	    
	    file(GLOB _libs "/usr/local/etc/vulkan")
	    foreach( _lib ${_libs} )
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
	    file(GLOB _libs "${HOME}/etc/vulkan/")
	    foreach( _lib ${_libs} )
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
	    file(GLOB _libs "/etc/vulkan/")
	    foreach( _lib ${_libs} )
		file(COPY ${_lib}
		    DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/etc/
		    FOLLOW_SYMLINK_CHAIN)
		set(_vulkan_found TRUE)
	    endforeach()
	    
	    if(NOT _vulkan_found)
		message(FATAL_ERROR "Could not locate /etc/vulkan/icd.d/*.json")
	    endif()
	endfunction()
	
	function(install_vulkan_layers APPNAME)
	    #
	    # Try Vulkan SDK first
	    #
	    set(_vulkan_found FALSE)
	    if (DEFINED VULKAN_SDK AND EXISTS ${VULKAN_SDK})
		file(GLOB _dirs "${VULKAN_SDK}/share/vulkan/*_layer.d")
		foreach( _dir ${_dirs} )
		    file(COPY ${_dir}
			DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/share/vulkan
			FOLLOW_SYMLINK_CHAIN)
		    set(_vulkan_found TRUE)
		endforeach()
		if(_vulkan_found)
		    return()
		endif()
		file(GLOB _dirs "${VULKAN_SDK}/etc/vulkan/*_layer.d")
		foreach( _dir ${_dirs} )
		    file(COPY ${_dir}
			DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/share/vulkan
			FOLLOW_SYMLINK_CHAIN)
		    set(_vulkan_found TRUE)
		endforeach()
		if(NOT _vulkan_found)
		    message(FATAL_ERROR "VULKAN_SDK set to ${VULKAN_SDK} but ${VULKAN_SDK}/share/vulkan/*layer.d not found")
		endif()
		return()
	    endif()

	    #
	    # Try /usr/local next
	    #
	    
	    file(GLOB _dirs "/opt/homebrew/vulkan-validationlayers/share/vulkan/*_layer.d")
	    foreach( _dir ${_dirs} )
		file(COPY ${_dir}
		    DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/share/vulkan
		    FOLLOW_SYMLINK_CHAIN)
		set(_vulkan_found TRUE)
	    endforeach()
	    if(_vulkan_found)
		message(STATUS "VULKAN_SDK set to ${VULKAN_SDK} but ${VULKAN_SDKw}/share/vulkan/*layer.d not found")
		return()
	    endif()

	    file(GLOB _dirs "/usr/local/opt/vulkan-validationlayers/share/vulkan/")
	    foreach( _dir ${_dirs} )
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
	    file(GLOB _dirs "${HOME}/share/vulkan/*_layer.d")
	    foreach( _dir ${_dirs} )
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
	    file(GLOB _dirs "/etc/vulkan/*_layer.d")
	    foreach( _dir ${_dirs} )
		file(COPY ${_dir}
		    DESTINATION ${CPACK_PREPACKAGE}/${APPNAME}.app/Contents/Resources/share/vulkan
		    FOLLOW_SYMLINK_CHAIN)
		set(_vulkan_found TRUE)
	    endforeach()
	    
	    if(NOT _vulkan_found)
		message(FATAL_ERROR "Could not locate vulkan/*_layer.d")
	    endif()
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
	set(VULKAN_SDK $ENV{VULKAN_SDK})
	message(STATUS "VULKAN_SDK set to ${VULKAN_SDK}")
	install_vulkan_lib_glob("libvulkan*" hdr)
	
	install_vulkan_icd_filenames(hdr)
	# install_vulkan_layers(hdr)


	# If MR2V2's BACKEND is VK, also install vulkan on it
	if (MRV2_BACKEND STREQUAL "VK" OR MRV2_BACKEND STREQUAL "BOTH")
	    install_vulkan_lib_glob("libvulkan*" mrv2)
	
	    install_vulkan_icd_filenames(mrv2)
	    # install_vulkan_layers(mrv2)
	endif()
	
	#
	# Install .mo translation files
	#
	copy_mo_files(hdr)
    endif()
    
    #
    # Clean up main staging area
    #
    file(REMOVE_RECURSE ${CPACK_PREPACKAGE}/bin)
    file(REMOVE_RECURSE ${CPACK_PREPACKAGE}/colors)
    file(REMOVE_RECURSE ${CPACK_PREPACKAGE}/docs)
    file(REMOVE_RECURSE ${CPACK_PREPACKAGE}/icons)
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
endif()
