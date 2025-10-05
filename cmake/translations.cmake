# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Language codes to be translated
#
set( LANGUAGES de en es fr hi_IN it ja pt ru zh-CN )

#
# This file creates the .pot / .po / .mo language translations
#
#

function(create_translation_for TARGET SOURCES)

    set( ROOT_DIR ${PROJECT_SOURCE_DIR} )
    
    # Create dir to place human translation files
    file( MAKE_DIRECTORY "${ROOT_DIR}/po/${TARGET}" )
    set( _absPotFile "${ROOT_DIR}/po/${TARGET}/messages.pot" )


    #
    # Get all python plug-ins and create output directory for python .pot files
    #
    file(GLOB _py_plugins "${ROOT_DIR}/python/plug-ins/*.py")
    set( _potPythonPluginDir "${ROOT_DIR}/po/python/plug-ins/locale")
    file(MAKE_DIRECTORY ${_potPythonPluginDir})

    #
    # Create a .pot file for each python plug-in
    #
    if (MRV2_PYBIND11 AND "${TARGET}" STREQUAL "mrv2")
	foreach(_py_plugin_full_path ${_py_plugins})
	    get_filename_component(_py_plugin ${_py_plugin_full_path} NAME)
	    get_filename_component(_py_basename ${_py_plugin} NAME_WLE)
	    set( _potFile "${_potPythonPluginDir}/${_py_basename}.pot" )

	    #
	    # First, try to find pygettext.py script
	    #
	    set(_pyscript_dir "${CMAKE_BINARY_DIR}/../../../deps/Python/src/Python/Tools/i18n" )
	    if (NOT EXISTS ${_pyscript_dir})
		find_package(Python COMPONENTS Interpreter)
		get_filename_component(_dir ${PYTHON_EXECUTABLE} DIRECTORY)
		set(_pyscript_dir "${_dir}/../share/doc/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/examples/Tools/i18n" )
	    endif()
	    
	    set(_py_gettext_script "${_pyscript_dir}/pygettext.py")

	    #
	    # If pygettext.py script not found, search for command
	    #
	    if (NOT EXISTS ${_py_gettext_script})
		message(WARNING "pygettext.py not found!  Looking for command")
		find_program(_py_gettext_cmd NAMES pygettext3 pygettext)
	    else()
		set(_py_gettext_cmd ${PYTHON_EXECUTABLE} )
	    endif()
	    
	    set(_py_gettext_args ${_py_gettext_script}
		-d ${_py_basename} -o ${_potFile} ${_py_plugin_full_path} )
	    
	    if (NOT DEFINED _py_gettext_cmd OR
		    "${_py_gettext_cmd}" STREQUAL "")
		message(FATAL_ERROR "pygettext command or pygettext.py script missing.  Did not know how to create .pot file ${_potFile}")
	    else()
		# If we have a pygettext command, create the plugin.pot file target
		add_custom_command(OUTPUT ${_potFile}
		    COMMAND ${_py_gettext_cmd} ${_py_gettext_args}
		    DEPENDS ${_py_plugin_full_path}
		    COMMENT "Creating ${_potFile}"
		)
		list(APPEND pot_files ${_potFile})
	    endif()
	endforeach()
    endif()
    
    #
    # Then, create the .po files if they don't exist and add a target for .mo files.
    #	   
    foreach( lang ${LANGUAGES} )
	
	set( _moDir "${ROOT_DIR}/share/locale/${lang}/LC_MESSAGES" )
	set( _moFile "${_moDir}/${TARGET}-v${mrv2_VERSION}.mo" )
	set( _poDir  "${ROOT_DIR}/po/${TARGET}" )
	set( _poFile "${_poDir}/${lang}.po" )

	if (NOT EXISTS ${_poFile} )
	    message( STATUS "${_poFile} does not exist.  Calling msginit" )
	    execute_process( COMMAND
		msginit --input=${_absPotFile} --no-translator --locale=${lang} --output=${_poFile} )
	endif()

	list(APPEND mo_files ${_moFile})
	list(APPEND po_files ${_poFile})

	#
	# We remove the mo dir as we version the .mo files, so we don't want to
	# leave old versions behind.
	#
	file( REMOVE_RECURSE "${_moDir}" ) # Remove dir to remove old .mo files
	file( MAKE_DIRECTORY "${_moDir}" ) # Recreate dir to place new .mo file


	add_custom_command( OUTPUT "${_moFile}"
	    COMMAND msgmerge --quiet --update --backup=none ${_poFile} "${_absPotFile}"
	    COMMAND msgfmt -v "${_poFile}" -o "${_moFile}"
	    DEPENDS ${_poFile} ${_absPotFile}
	    COMMENT "Creating ${_moFile} after merging ${_poFile}"
	)
	
	#
	# Create py plugins translations
	#
	if (MRV2_PYBIND11 AND ${TARGET} STREQUAL "mrv2")

	    #
	    # Install plugin translation files
	    #
	    install( DIRECTORY ${ROOT_DIR}/python/plug-ins/locale/${lang}
		DESTINATION python/plug-ins/locale
		COMPONENT applications )

	    
	    set( _poDir "${ROOT_DIR}/po/${TARGET}/python/plug-ins/locale/${lang}/LC_MESSAGES" )
	    set( _moDir "${ROOT_DIR}/python/plug-ins/locale/${lang}/LC_MESSAGES" )
	    file( MAKE_DIRECTORY "${_poDir}" ) # Recreate dir to place new .po file
	    file( MAKE_DIRECTORY "${_moDir}" ) # Recreate dir to place new .mo file
	
	    foreach(_py_plugin_full_path ${_py_plugins})
		get_filename_component(_py_plugin ${_py_plugin_full_path} NAME)
		get_filename_component(_py_basename ${_py_plugin} NAME_WLE)
		set( _moFile  "${_moDir}/${_py_basename}.mo" )
		set( _poFile  "${_poDir}/${_py_basename}.po" )
		set( _potFile "${_potPythonPluginDir}/${_py_basename}.pot" )

		if (NOT EXISTS ${_poFile})
		    message( STATUS "${_poFile} does not exist.  Calling msginit" )
		    execute_process( COMMAND
			msginit --input=${_potFile} --no-translator --locale=${lang} --output=${_poFile} )
		endif()
	
		add_custom_command( OUTPUT "${_poFile}"
		    COMMAND msgmerge --lang ${lang} --quiet --update --backup=none "${_poFile}" "${_potFile}"
		    DEPENDS ${_potFile}
		    COMMENT "Creating ${_poFile} after merging ${_potFile}"
		)
	    
		add_custom_command( OUTPUT "${_moFile}"
		    COMMAND msgfmt -v "${_poFile}" -o "${_moFile}"
		    DEPENDS ${_poFile}
		    COMMENT "Creating ${_moFile}"
		)
		
		list(APPEND pot_files ${_potFile})
		list(APPEND po_files ${_poFile})
		list(APPEND mo_files ${_moFile})
	    endforeach()
	endif()
    endforeach()

    #
    # Only make the .pot file depend on Unix machines to avoid issues with
    # different gettext implementations/versions on different OSes which would
    # lead to lots of .pot commits.
    #
    set(_abspot_dep )
    set(_pot_target ${TARGET}_pot)
    if(UNIX AND NOT APPLE)
	set(_abspot_dep ${TARGET} )
    endif()
    add_custom_target(
	${_pot_target}
	COMMAND xgettext --package-name=${TARGET} --package-version="v${mrv2_VERSION}" --copyright-holder="Contributors to the mrv2 Project" --msgid-bugs-address="ggarra13@gmail.com" -d ${TARGET} -c++ -k_ ${SOURCES} -o "${_absPotFile}"
	WORKING_DIRECTORY "${ROOT_DIR}/lib"
	COMMENT Running xgettext for ${TARGET}_pot target
	DEPENDS ${_abspot_dep}
    )

    #
    # Send the variables to the PARENT_SCOPE (ie. outside the function)
    #
    set(pot_files ${pot_files} PARENT_SCOPE)
    set(po_files ${po_files} PARENT_SCOPE)
    set(mo_files ${mo_files} PARENT_SCOPE)
    
endfunction()



set( pot_files )
set( po_files )
set( mo_files )
    
create_translation_for(mrv2 "${PO_SOURCES}")

set(hdr_pot_target )
if (TLRENDER_NDI AND TLRENDER_VK AND MRV2_HDR)
    create_translation_for(hdr "${PO_HDR_SOURCES}")
    set(hdr_pot_target hdr_pot)
endif()



add_custom_target(
    pot
    DEPENDS ${pot_files} mrv2_pot ${hdr_pot_target}
)

add_custom_target(
    po
    DEPENDS ${po_files} pot
    )


add_custom_target(
    mo
    DEPENDS ${mo_files} po mrv2_pot ${hdr_pot_target}
    )
