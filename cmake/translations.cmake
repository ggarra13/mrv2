# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
# This file contains the .pot / .po / .mo language translations
#
#

set( ROOT_DIR ${PROJECT_SOURCE_DIR} )

 # Create dir to place human translation files
file( MAKE_DIRECTORY "${ROOT_DIR}/po" )
set( _absPotFile "${ROOT_DIR}/po/messages.pot" )


set( LANGUAGES "en" "es" )

set( output_files "${_absPotFile}" )


foreach( lang ${LANGUAGES} )

  set( _moDir "${ROOT_DIR}/share/locale/${lang}/LC_MESSAGES/" )
  set( _moFile "${_moDir}/mrv2-v${mrv2_VERSION}.mo" )
  set( _absFile "${ROOT_DIR}/po/${lang}.po" )

  if (NOT EXISTS ${_absFile} )
      execute_process( COMMAND ${CMAKE_COMMAND} -E touch ${_absFile} )
  endif()

  set( output_files ${output_files} ${_moFile} )

  file( REMOVE_RECURSE "${_moDir}" ) # Remove dir to remove old .mo files
  file( MAKE_DIRECTORY "${_moDir}" ) # Recreate dir to place new .mo file

  add_custom_command( OUTPUT "${_moFile}"
      COMMAND msgmerge --quiet --update --backup=none
      "${_absFile}" "${_absPotFile}"
      COMMAND msgfmt -v "${_absFile}" -o "${_moFile}"
      DEPENDS ${_absFile} ${_absPotFile}
  )

endforeach()


add_custom_command( OUTPUT "${_absPotFile}"
    COMMAND xgettext
    ARGS --package-name=mrv2 --package-version="${mrv2_VERSION}" --copyright-holder="Contributors to the mrv2 Project" --msgid-bugs-address=ggarra13@gmail.com -d mrv2 --sort-output -c++ -k_ ${PO_SOURCES} -o "${_absPotFile}"
    DEPENDS mrv2
)

add_custom_target(
     po
     DEPENDS ${output_files} ${PROJECT_NAME}
)
