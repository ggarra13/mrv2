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

#
# We must list all libraries so that old gcc compiler on Docker will link
# the stdc++fs library properly. 
#
set(LIBRARIES )

if(MRV2_PYBIND11)
    list(APPEND LIBRARIES mrvPy)
endif()

list( APPEND LIBRARIES
    mrvApp
    ${Intl_LIBRARIES} )



if( APPLE )
    set( OSX_FRAMEWORKS "-framework Cocoa -framework IOKit" )
    list( APPEND LIBRARIES ${OSX_FRAMEWORKS} )
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;/usr/local/lib")
endif()

if(UNIX AND NOT APPLE)
    if( CMAKE_CXX_COMPILER_ID MATCHES "GNU" )
	if ( CMAKE_CXX_COMPILER_VERSION MATCHES "^8.*" )
	    message( STATUS "Old g++ compiler detected.  Will link to stdc++fs")
	    list( APPEND LIBRARIES "stdc++fs" )
	endif()
	list( APPEND LIBRARIES "util" )  # Needed for python3.10 on Rocky
    endif()
endif()

add_executable(mrv2 WIN32 ${SOURCES} ${HEADERS})

target_link_libraries(mrv2 PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories( mrv2 BEFORE PUBLIC ${CMAKE_INSTALL_PREFIX}/lib /usr/local/lib )

set(PRUNE_SYMBOLS )
if(WIN32)
    set(PRUNE_SYMBOLS "/OPT:REF" "/OPT:ICF")
elseif(APPLE)
    set(PRUNE_SYMBOLS "-Wl,-dead_strip")
else(UNIX)
    set(PRUNE_SYMBOLS "-Wl,--gc-sections")
endif()

target_link_options(mrv2 PRIVATE ${PRUNE_SYMBOLS})

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


#
# Extract python version from python executable
#
string( REGEX MATCH "^[0-9]+\\.[0-9]+" MRV2_PYTHON_VERSION "${PYTHON_VERSION}" )

if (UNIX)
    set( MRV2_PYTHON_LIBS ${CMAKE_INSTALL_PREFIX}/lib/python${MRV2_PYTHON_VERSION} )
    install( DIRECTORY ${MRV2_PYTHON_LIBS}
        DESTINATION lib
        COMPONENT applications )
endif()

if ( APPLE )
#Install all.dylibs in CMAKE_INSTALL_PREFIX / lib
    file( GLOB MRV_DYLIBS ${CMAKE_INSTALL_PREFIX}/lib/*.dylib* )
    install( FILES ${MRV_DYLIBS}
	DESTINATION lib
	COMPONENT applications )
elseif ( UNIX )
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
	# Install all .dll in CMAKE_INSTALL_PREFIX/bin
	file( GLOB MRV_PYLib ${CMAKE_INSTALL_PREFIX}/bin/Lib )
	file( GLOB MRV_PYDLLs ${CMAKE_INSTALL_PREFIX}/bin/DLLs )
	file( GLOB MRV_PYTCL ${CMAKE_INSTALL_PREFIX}/bin/tcl  )
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

    # For Python, Poco and FFmpeg
    file( GLOB MRV_DLLS ${CMAKE_INSTALL_PREFIX}/bin/*.dll  )
    install( FILES ${MRV_DLLS}
    	     DESTINATION bin/
	     COMPONENT applications )
	    
endif()


if ( NOT WIN32 )
    # Configure environment.sh with python path
    configure_file(${CMAKE_SOURCE_DIR}/bin/environment.sh.in
	${CMAKE_INSTALL_PREFIX}/bin/environment.sh @ONLY)
    
    install( FILES
	${CMAKE_SOURCE_DIR}/bin/mrv2.sh
	${CMAKE_INSTALL_PREFIX}/bin/environment.sh
	DESTINATION bin
        PERMISSIONS 
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
	COMPONENT applications )
    
    if(TLRENDER_USD)
	install( DIRECTORY
	    ${CMAKE_INSTALL_PREFIX}/lib/usd
	    DESTINATION lib
	    COMPONENT applications)
    endif()
else()
    if(TLRENDER_USD)
	install( DIRECTORY
	    ${CMAKE_INSTALL_PREFIX}/bin/usd
	    DESTINATION bin
	    COMPONENT applications)
    endif()
endif()


#
# USD dependencies
#
if(TLRENDER_USD)
    install( DIRECTORY
        ${CMAKE_INSTALL_PREFIX}/plugin
        ${CMAKE_INSTALL_PREFIX}/libraries
        ${CMAKE_INSTALL_PREFIX}/resources
        DESTINATION .
        COMPONENT applications)
endif()

