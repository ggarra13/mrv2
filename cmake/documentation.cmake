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

    #Replace variables inside @@ with the current values
    configure_file(${CONFFILE_IN} ${CONFFILE_OUT} @ONLY)

    #
    # Replace variables in document.py script
    #
    
    set(DOCUMENT_IN  ${SPHINX_DIR}/document.py.in)
    set(DOCUMENT_OUT ${SPHINX_DIR}/${LANGUAGE}/document.py)

    #Replace variables inside @@ with the current values
    configure_file(${DOCUMENT_IN} ${DOCUMENT_OUT} @ONLY)

    set( DOC_TARGET doc_${LANGUAGE} )
    list( APPEND DOCUMENTATION_TARGETS ${DOC_TARGET} )
    
    add_custom_target( ${DOC_TARGET}
	COMMAND ${CMAKE_COMMAND} -E env LANGUAGE=${LANGUAGE} ${CMAKE_INSTALL_PREFIX}/bin/${MRV2_COMMAND} -pythonScript ${SPHINX_DIR}/${LANGUAGE}/document.py
	DEPENDS mrv2
	)
    
endforeach()

add_custom_target( doc
    COMMAND echo "Documenting all languages..."
    DEPENDS ${DOCUMENTATION_TARGETS}
    )
