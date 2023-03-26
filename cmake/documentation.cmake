# Prepare Sphinx documentation.

set( RELATIVE_ROOT_DIR   ${PROJECT_SOURCE_DIR}/.. )
file( REAL_PATH ${RELATIVE_ROOT_DIR} ROOT_DIR )


# Point to Sphinx directory 
set( SPHINX_DIR ${ROOT_DIR}/docs/sphinx )

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
    set(DOCUMENT_OUT ${SPHINX_DIR}/document_${LANGUAGE}.py)

    #Replace variables inside @@ with the current values
    configure_file(${DOCUMENT_IN} ${DOCUMENT_OUT} @ONLY)

endforeach()
