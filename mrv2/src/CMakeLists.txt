# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


set(HEADERS )
set(SOURCES main.cpp )

files_to_absolute_paths()
set( PO_SOURCES ${PO_FILES} ${PO_SOURCES} PARENT_SCOPE)

if (WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")
    list( APPEND SOURCES ${APP_ICON_RESOURCE_WINDOWS} )
endif()

list( APPEND LIBRARIES
    mrvPy
    mrvApp
    mrvPanels
    mrvFl
    mrvGL
    mrvWidgets
    mrvFLU
    mrvDraw
    mrvCore
    tlGL
    ${Intl_LIBRARIES} )


if( APPLE )
    set( OSX_FRAMEWORKS "-framework Cocoa -framework IOKit" )
    list( APPEND LIBRARIES ${OSX_FRAMEWORKS} )
endif()

if(UNIX AND NOT APPLE)
    if( CMAKE_CXX_COMPILER_ID MATCHES "GNU" )
	if ( CMAKE_CXX_COMPILER_VERSION MATCHES "^8.*" )
	    message( STATUS "Old g++ compiler detected.  Will link to stdc++fs")
	    list( APPEND LIBRARIES "stdc++fs" )
	endif()
    endif()
endif()


add_executable(mrv2 WIN32 ${SOURCES} ${HEADERS})

target_link_libraries(mrv2 PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories( mrv2 BEFORE PRIVATE ${CMAKE_PREFIX_PATH}/lib )

install(TARGETS mrv2
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION lib COMPONENT libraries
    ARCHIVE DESTINATION lib COMPONENT libraries )

#
# Handle translations
#
include( ../../cmake/translations.cmake )

#
# Handle documentation
#
include( ../../cmake/documentation.cmake )


#
# Handle install
#
include(InstallRequiredSystemLibraries)

file( REAL_PATH "${PYTHON_EXECUTABLE}" MRV2_PYTHON_EXECUTABLE )
string( REGEX REPLACE "(.*/)(python.*)" \\2 MRV2_PYTHON_VERSION "${MRV2_PYTHON_EXECUTABLE}" )   

if ( APPLE )
    # Install all .dylibs in CMAKE_INSTALL_PREFIX/lib
    file( GLOB MRV_DYLIBS ${CMAKE_INSTALL_PREFIX}/lib/*.dylib* )
    install( FILES ${MRV_DYLIBS}
	DESTINATION lib
	COMPONENT applications )
elseif ( UNIX )
    #
    # Handle python DSOs and stdlib
    #
    if(MRV2_PYBIND11)      
	string( REGEX REPLACE "(.*)/bin/python.*" \\1 MRV2_PYTHON_ROOT "${PYTHON_EXECUTABLE}" )
	set( MRV2_PYTHON_LIBDIR "${MRV2_PYTHON_ROOT}/lib/${MRV2_PYTHON_VERSION}" )
	message( STATUS "Copying Python ${MRV2_PYTHON_LIBDIR} to ${CMAKE_INSTALL_PREFIX}/lib/" )

	install( DIRECTORY ${MRV2_PYTHON_LIBDIR}
	    DESTINATION lib/
	    COMPONENT applications )
	
	set( MRV2_PYTHON_LIB64DIR "${MRV2_PYTHON_ROOT}/lib64/${MRV2_PYTHON_VERSION}" )
	message( STATUS "Copying Python ${MRV2_PYTHON_LIB64DIR} to ${CMAKE_INSTALL_PREFIX}/lib/" )
	if( EXISTS ${MRV2_PYTHON_LIB64DIR} )
	    install( DIRECTORY ${MRV2_PYTHON_LIB64DIR}
		DESTINATION lib/
		COMPONENT applications )
	endif()
    endif()
    
    # Install all .so in CMAKE_INSTALL_PREFIX/lib
    file( GLOB MRV_DSOS ${CMAKE_INSTALL_PREFIX}/lib/*.so* )
    install( FILES ${MRV_DSOS}
      DESTINATION lib
      COMPONENT applications )
elseif(WIN32)
    #
    # Handle python DLL and libs and TCL
    #
    if(MRV2_PYBIND11)
	string( REGEX REPLACE "python.exe" "" PYTHON_ROOT
	    "${PYTHON_EXECUTABLE}" )
	message( STATUS "Copying Python ${PYTHON_ROOT} to ${CMAKE_INSTALL_PREFIX}/bin/" )
	file( GLOB DEPENDENCIES "${PYTHON_ROOT}/*.dll" )
	set( DEPENDENCIES "${PYTHON_ROOT}/DLLs" "${PYTHON_ROOT}/Lib"
	    "${PYTHON_ROOT}/tcl" ${DEPENDENCIES} )
	file( COPY ${DEPENDENCIES} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/" )
	
	# Install all .dll in CMAKE_INSTALL_PREFIX/bin
	file( GLOB MRV_DLLS ${CMAKE_INSTALL_PREFIX}/bin/*.dll  )
	file( GLOB MRV_PYLib ${CMAKE_INSTALL_PREFIX}/bin/Lib )
	file( GLOB MRV_PYDLLs ${CMAKE_INSTALL_PREFIX}/bin/DLLs )
	file( GLOB MRV_PYTCL ${CMAKE_INSTALL_PREFIX}/bin/tcl  )
	install( FILES ${MRV_DLLS}
	    DESTINATION bin
	    COMPONENT applications )
	install( DIRECTORY ${MRV_PYLib}
	    DESTINATION bin/
	    COMPONENT applications )
	install( DIRECTORY ${MRV_PYDLLs}
	    DESTINATION bin/
	    COMPONENT applications )
	install( DIRECTORY ${MRV_PYTCL}
	    DESTINATION bin/
	    COMPONENT applications )
    endif()

endif()


if ( NOT WIN32 )
    # COnfigure environment.sh with python path
    configure_file(${CMAKE_SOURCE_DIR}/bin/environment.sh.in
	${CMAKE_SOURCE_DIR}/bin/environment.sh @ONLY)
    
    install( FILES ${CMAKE_SOURCE_DIR}/bin/mrv2.sh
	${CMAKE_SOURCE_DIR}/bin/environment.sh
	DESTINATION bin
	COMPONENT applications )
endif()
