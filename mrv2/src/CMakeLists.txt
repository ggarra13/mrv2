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
# Handle install
#
include(InstallRequiredSystemLibraries)

if ( NOT WIN32 )
    install( FILES ${CMAKE_SOURCE_DIR}/bin/mrv2.sh
	${CMAKE_SOURCE_DIR}/bin/environment.sh
	DESTINATION bin
	COMPONENT applications )
endif()

if ( APPLE )
    # Install all .dylibs in CMAKE_INSTALL_PREFIX/lib
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
else()
    # Install all .dll in CMAKE_INSTALL_PREFIX/bin
    file( GLOB MRV_DLLS ${CMAKE_INSTALL_PREFIX}/bin/*.dll )
    install( FILES ${MRV_DLLS}
	DESTINATION bin
	COMPONENT applications )
endif()
