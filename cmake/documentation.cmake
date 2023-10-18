# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

# Prepare Sphinx documentation.

set( RELATIVE_ROOT_DIR   ${PROJECT_SOURCE_DIR}/.. )
file( REAL_PATH ${RELATIVE_ROOT_DIR} ROOT_DIR )


# Point to Sphinx directory 
set( SPHINX_DIR ${ROOT_DIR}/docs/sphinx )

set( MRV2_COMMAND mrv2 )
if (UNIX)
    set( MRV2_COMMAND mrv2.sh )
endif()
set( DOCUMENTATION_TARGETS )


foreach( LANGUAGE ${LANGUAGES} )
    
    #
    # Prepare Sphinx's conf.py
    #
    set(CONFFILE_IN ${SPHINX_DIR}/${LANGUAGE}/conf.py.in)
    set(CONFFILE_OUT ${SPHINX_DIR}/${LANGUAGE}/conf.py)

    # Replace variables inside @@ with the current values
    file( REMOVE ${CONFFILE_OUT} )
    message( STATUS "Creating ${CONFFILE_OUT}..." )
    configure_file(${CONFFILE_IN} ${CONFFILE_OUT} @ONLY)
    

    #
    # Replace variables in document.py script
    #

    set(DOCUMENT_IN  ${SPHINX_DIR}/document.py.in)
    set(DOCUMENT_OUT ${CMAKE_INSTALL_PREFIX}/sphinx/${LANGUAGE}/document.py)

    file( REMOVE ${DOCUMENT_OUT} )
    
    # Replace variables inside @@ with the current values
    message( STATUS "Creating ${DOCUMENT_OUT}..." )
    configure_file(${DOCUMENT_IN} ${DOCUMENT_OUT} @ONLY)

    # Make sure document.py is executable
    file(CHMOD ${DOCUMENT_OUT}
        FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)
    
    set( DOC_TARGET doc_${LANGUAGE} )
    list( APPEND DOCUMENTATION_TARGETS ${DOC_TARGET} )
    
    add_custom_target( ${DOC_TARGET}
	COMMAND ${CMAKE_COMMAND} -E env LANGUAGE=${LANGUAGE} LANG=en_US.UTF-8 LANGUAGE_CODE=${LANGUAGE} ${CMAKE_INSTALL_PREFIX}/bin/${MRV2_COMMAND} -pythonScript ${CMAKE_INSTALL_PREFIX}/sphinx/${LANGUAGE}/document.py
	DEPENDS install ${DOCUMENT_OUT}
	)
    
endforeach()

add_custom_target( doc
    COMMAND ${CMAKE_COMMAND} -E echo "Documented all languages."
    DEPENDS ${DOCUMENTATION_TARGETS}
    )
