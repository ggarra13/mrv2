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

foreach( lang ${LANGUAGES} )

    set( _moDir "${ROOT_DIR}/share/locale/${lang}/LC_MESSAGES/" )
    set( _moFile "${_moDir}/mrv2-v${mrv2_VERSION}.mo" )
    set( _poFile "${ROOT_DIR}/po/${lang}.po" )

    if (NOT EXISTS ${_poFile} )
	message( STATUS "${_poFile} does not exist.  Calling msginit" )
	execute_process( COMMAND
	    msginit --input=${_absPotFile} --locale=${lang} --output=${_poFile} )
    endif()

    list(APPEND mo_files ${_moFile} )

    file( REMOVE_RECURSE "${_moDir}" ) # Remove dir to remove old .mo files
    file( MAKE_DIRECTORY "${_moDir}" ) # Recreate dir to place new .mo file


    add_custom_command( OUTPUT "${_moFile}"
	COMMAND msgmerge --quiet --update --backup=none ${_poFile} "${_absPotFile}"
	COMMAND msgfmt -v "${_poFile}" -o "${_moFile}"
	DEPENDS ${_poFile} ${_absPotFile}
    )

    file(GLOB _py_plugins "${ROOT_DIR}/python/plug-ins/*.py")
    foreach(_full_path ${_py_plugins})
	get_filename_component(_py_plugin ${_full_path} NAME)
	get_filename_component(_py_basename ${_py_plugin} NAME_WLE)
	message( STATUS "Translating py plugin ${_py_plugin} into ${lang}")
	
	set( _moDir "${ROOT_DIR}/python/plug-ins/locales/${lang}/LC_MESSAGES" )
	set( _moFile "${_moDir}/${_py_basename}.mo" )
	set( _poFile "${_moDir}/${_py_basename}.po" )

	message( STATUS "check ${_poFile}")
	if (NOT	EXISTS ${_poFile})
	    execute_process( COMMAND
		${CMAKE_COMMAND} -E make_directory ${_moDir}
		COMMAND
		${CMAKE_COMMAND} -E echo "pygettext3 -d ${py_basename} -o ${_poFile} ${_full_path}"
		COMMAND
		pygettext3 -d ${_py_basename} -o ${_poFile} ${_full_path} )
	endif()
	
	add_custom_command( OUTPUT "${_moFile}"
	    COMMAND msgfmt -v "${_poFile}" -o "${_moFile}"
	    DEPENDS ${_poFile} ${_full_path}
	)
	
	list(APPEND mo_files ${_moFile} )
    endforeach()

endforeach()




add_custom_target(
    pot
    COMMAND echo xgettext
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
