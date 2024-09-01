# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

# Prepare Sphinx documentation.

set( RELATIVE_ROOT_DIR   ${PROJECT_SOURCE_DIR}/.. )
file( REAL_PATH ${RELATIVE_ROOT_DIR} ROOT_DIR )


# Point to Sphinx directory 
set(SPHINX_DIR ${ROOT_DIR}/docs/sphinx )

set(MRV2_COMMAND mrv2 )
if (UNIX)
    set(MRV2_COMMAND mrv2.sh )
endif()
set(DOCUMENTATION_TARGETS )
set(CLEAN_DOC_TARGETS )

set(DOC_LANGUAGES en es)

if (NOT DOC_LANGUAGES)
    set(DOC_LANGUAGES ${LANGUAGES})
endif()

foreach( LANGUAGE ${DOC_LANGUAGES} )
    
    #
    # Prepare Sphinx's conf.py
    #
    set(CONFFILE_IN ${SPHINX_DIR}/${LANGUAGE}/conf.py.in)
    set(CONFFILE_OUT ${SPHINX_DIR}/${LANGUAGE}/conf.py)

    # Replace variables inside @@ with the current values
    file( REMOVE ${CONFFILE_OUT} )

    # Replace variables inside @@ with the current values

    if(EXISTS ${CONFFILE_IN})
	message( DEBUG "Creating ${CONFFILE_OUT}..." )
	configure_file(${CONFFILE_IN} ${CONFFILE_OUT} @ONLY)
    endif()

    #
    # Replace variables in document.py script
    #
    set(DOCUMENT_IN  ${SPHINX_DIR}/document.py.in)
    set(DOCUMENT_OUT ${CMAKE_INSTALL_PREFIX}/sphinx/${LANGUAGE}/document.py)

    file( REMOVE ${DOCUMENT_OUT} )
    
    # Replace variables inside @@ with the current values
    message( DEBUG "Creating ${DOCUMENT_OUT}..." )
    configure_file(${DOCUMENT_IN} ${DOCUMENT_OUT} @ONLY)

    # Make sure document.py is executable
    file(CHMOD ${DOCUMENT_OUT}
        FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)

    if (MRV2_PYBIND11)
	set(DOC_TARGET doc_${LANGUAGE} )
	list(APPEND DOCUMENTATION_TARGETS ${DOC_TARGET} )

	set(CLEAN_DOC_TARGET clean_doc_dir_${LANGUAGE})
	add_custom_target(${CLEAN_DOC_TARGET}
	    COMMAND ${CMAKE_COMMAND} -E remove_directory
	    ${ROOT_DIR}/src/docs/${LANGUAGE}
	)
	list( APPEND CLEAN_DOC_TARGETS ${CLEAN_DOC_TARGET} )

	add_custom_target( ${DOC_TARGET}
	    COMMAND ${CMAKE_COMMAND} -E env LANGUAGE=${LANGUAGE} LANGUAGE_CODE=${LANGUAGE} ${CMAKE_INSTALL_PREFIX}/bin/${MRV2_COMMAND} -pythonScript ${CMAKE_INSTALL_PREFIX}/sphinx/${LANGUAGE}/document.py
	    COMMAND ${CMAKE_COMMAND} -E echo "Documented ${LANGUAGE} language."
	    DEPENDS ${DOCUMENT_OUT} ${CONFFILE_OUT}
	)
    endif()
endforeach()

add_custom_target( clean_doc
    COMMAND ${CMAKE_COMMAND} -E echo "Cleaned all languages documentations."
    DEPENDS ${CLEAN_DOC_TARGETS} 
)

add_custom_target( doc
    COMMAND ${CMAKE_COMMAND} -E echo "Documented all languages."
    DEPENDS ${DOCUMENTATION_TARGETS} clean_doc
)


if(MRV2_PYBIND11)
    add_custom_target( pip
	COMMAND ${PYTHON_EXECUTABLE} -m pip install --upgrade pip
    )

    if(UNIX)
	set(_install_requests_cmd ${PYTHON_EXECUTABLE} -m pip install
	    --upgrade 
	    --target
	    ${CMAKE_INSTALL_PREFIX}/lib/python${MRV2_PYTHON_VERSION}/site-packages
	    requests)
    else()
	set(_install_requests_cmd ${PYTHON_EXECUTABLE} -m pip install --upgrade --target ${CMAKE_INSTALL_PREFIX}/bin/Lib/site-packages requests)
    endif()

    add_custom_target( requests
	COMMAND ${_install_requests_cmd}
	DEPENDS pip mrv2
    )
endif()
