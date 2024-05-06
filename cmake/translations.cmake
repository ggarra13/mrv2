# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Language codes to be translated
#
set( LANGUAGES en es )

#
# This file creates the .pot / .po / .mo language translations
#
#

set( ROOT_DIR ${PROJECT_SOURCE_DIR} )

# Create dir to place human translation files
file( MAKE_DIRECTORY "${ROOT_DIR}/po" )
set( _absPotFile "${ROOT_DIR}/po/messages.pot" )



set( mo_files ${_absPotFile} )


#
# Get all python plug-ins and create output directory for python .pot files
#
file(GLOB _py_plugins "${ROOT_DIR}/python/plug-ins/*.py")
set( _potPythonPluginDir "${ROOT_DIR}/po/python/plug-ins/locales")
file(MAKE_DIRECTORY ${_potPythonPluginDir})

#
# Create a .pot file for each python plug-in
#
if (MRV2_PYBIND11)
    foreach(_full_path ${_py_plugins})
	get_filename_component(_py_plugin ${_full_path} NAME)
	get_filename_component(_py_basename ${_py_plugin} NAME_WLE)
	message( STATUS "Creating .pot file for plug-in ${_py_plugin}")
	set( _potFile "${_potPythonPluginDir}/${_py_basename}.pot" )

	#
	# First, try to find pygettext.py sript
	#
	set(_pyscript_dir "${CMAKE_BINARY_DIR}/../../../Python-prefix/src/Python/Tools/i18n" )
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
	    -d ${_py_basename} -o ${_potFile} ${_full_path} )
    
	if (NOT DEFINED _py_gettext_cmd OR
		"${_py_gettext_cmd}" STREQUAL "")
	    message(FATAL_ERROR "pygettext command or pygettext.py script missing.  Did not know how to create .pot file ${_potFile}")
	else()
	    # If we have a pygettext command, create the plugin.pot file target
	    add_custom_command(OUTPUT ${_potFile}
		COMMAND ${CMAKE_COMMAND} -E echo ${_py_gettext_cmd} ${_py_gettext_args}
		COMMAND ${_py_gettext_cmd} ${_py_gettext_args}
		DEPENDS ${_fullpath}
	    )
	endif()
    endforeach()
endif()


#
# Then, create the .po files if they don't exist and add a target for .mo files.
#	   
foreach( lang ${LANGUAGES} )

    set( _moDir "${ROOT_DIR}/share/locale/${lang}/LC_MESSAGES" )
    set( _moFile "${_moDir}/mrv2-v${mrv2_VERSION}.mo" )
    set( _poDir  "${ROOT_DIR}/po" )
    set( _poFile "${_poDir}/${lang}.po" )

    if (NOT EXISTS ${_poFile} )
	message( STATUS "${_poFile} does not exist.  Calling msginit" )
	execute_process( COMMAND
	    msginit --input=${_absPotFile} --no-translator --locale=${lang} --output=${_poFile} )
    endif()

    list(APPEND mo_files ${_moFile} )

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
    )
    
    #
    # Create py plugins translations
    #
    if (MRV2_PYBIND11)
	set( _poDir "${ROOT_DIR}/po/python/plug-ins/locales/${lang}/LC_MESSAGES" )
	set( _moDir "${ROOT_DIR}/python/plug-ins/locales/${lang}/LC_MESSAGES" )
	file( MAKE_DIRECTORY "${_poDir}" ) # Recreate dir to place new .po file
	file( MAKE_DIRECTORY "${_moDir}" ) # Recreate dir to place new .mo file

	foreach(_full_path ${_py_plugins})
	    get_filename_component(_py_plugin ${_full_path} NAME)
	    get_filename_component(_py_basename ${_py_plugin} NAME_WLE)
	    message( STATUS "Translating py plugin ${_py_plugin} into ${lang}")
	    set( _moFile  "${_moDir}/${_py_basename}.mo" )
	    set( _poFile  "${_poDir}/${_py_basename}.po" )
	    set( _potFile "${_potPythonPluginDir}/${_py_basename}.pot" )

	    if (NOT EXISTS ${_poFile})
		message( STATUS "${_poFile} does not exist.  Calling msginit" )
		execute_process( COMMAND
		    msginit --input=${_potFile} --no-translator --locale=${lang} --output=${_poFile} )
	    endif()
	
	    add_custom_command( OUTPUT "${_moFile}"
		COMMAND msgmerge --lang ${lang} --quiet --update --backup=none "${_poFile}" "${_potFile}"
		COMMAND msgfmt -v "${_poFile}" -o "${_moFile}"
		DEPENDS ${_poFile} ${_potFile}
	    )
	
	    list(APPEND mo_files ${_moFile} )
	endforeach()
    endif()

endforeach()




add_custom_target(
    pot
    COMMAND ${CMAKE_COMMAND} -E echo Running xgettext for pot target
    COMMAND xgettext --package-name=mrv2 --package-version="v${mrv2_VERSION}" --copyright-holder="Contributors to the mrv2 Project" --msgid-bugs-address="ggarra13@gmail.com" -d mrv2 -c++ -k_ ${PO_SOURCES} -o "${_absPotFile}"
    WORKING_DIRECTORY "${ROOT_DIR}/lib"
    # No dependency on any sources, so we don't update on any file change
    )



add_custom_target(
    po
    )


add_custom_target(
    mo
    DEPENDS ${mo_files}
    )
