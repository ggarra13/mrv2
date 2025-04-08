# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


set(HEADERS
    mrvContextObject.h
    mrvEnv.h
    mrvHDRApp.h
    mrvRoot.h
)
set(SOURCES
    mrvContextObject.cpp
    mrvEnv.cpp
    mrvHDRApp.cpp
    mrvRoot.cpp
    main.cpp)

hdr_files_to_absolute_paths()
set( PO_HDR_SOURCES ${PO_HDR_FILES} ${PO_HDR_SOURCES} PARENT_SCOPE)

find_package(Intl REQUIRED)

if (WIN32)
    enable_language("RC")
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/app.rc")
    list( APPEND SOURCES ${APP_ICON_RESOURCE_WINDOWS} )
endif()

set(LIBRARIES
    mrvBaseApp
    mrvHDR
    mrvHDRWidgets
    ${Intl_LIBRARIES})

if (WIN32)
    list(APPEND LIBRARIES ${FLTK_LIBRARIES})
else()
    if(FLTK_BUILD_SHARED_LIBS)
	list(APPEND LIBRARIES ${FLTK_LIBRARIES} fltk::vk-shared)
    else()
	list(APPEND LIBRARIES ${FLTK_LIBRARIES} fltk::vk)
    endif()
endif()

if( APPLE )
    set(OSX_FRAMEWORKS "-framework IOKit")
    list(APPEND LIBRARIES ${OSX_FRAMEWORKS})
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;/usr/local/lib")
endif()

add_executable(hdr WIN32 ${SOURCES} ${HEADERS})

target_include_directories( hdr BEFORE PRIVATE . )

target_link_libraries(hdr PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories( hdr BEFORE PUBLIC ${CMAKE_INSTALL_PREFIX}/lib /usr/local/lib )

install(TARGETS hdr
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION lib COMPONENT libraries
    ARCHIVE DESTINATION lib COMPONENT libraries )

#
# Handle translations
#
# include( ../../cmake/translations.cmake )

#
# Handle documentation
# 
# include( ../../cmake/documentation.cmake )


#
# Handle install
#
include(InstallRequiredSystemLibraries)


if(TLRENDER_NDI)
    # NDI libraries have two names: Advanced and normal.
    # We use a glob to simplify and also for potential future development
    # of NDI's SDK.
    if (WIN32)
	if (TLRENDER_NDI_SDK MATCHES ".*Advanced.*")
	    # Advanced lib is ${TLRENDER_NDI_SDK}/bin/x64/Processing.NDI.Lib.Advanced.x64.dll
	    file( GLOB HDR_LIB_DYLIBS ${TLRENDER_NDI_SDK}/bin/x64/Processing.NDI.Lib.Advanced.x64.dll )
	else()
	    # Normal lib is ${TLRENDER_NDI_SDK}/bin/x64/Processing.NDI.Lib.x64.dll
	    file( GLOB HDR_LIB_DYLIBS ${TLRENDER_NDI_SDK}/bin/x64/Processing.NDI.Lib.x64.dll )
	endif()
	install( FILES  ${HDR_LIB_DYLIBS}
	    DESTINATION bin
	    COMPONENT applications )
    elseif (APPLE)
	file( GLOB HDR_LIB_DYLIBS ${TLRENDER_NDI_SDK}/lib/macOS/*.dylib* )
	install( FILES ${HDR_LIB_DYLIBS}
	    DESTINATION lib
	    COMPONENT applications )
    elseif(UNIX)
	file( GLOB HDR_LIB_DSOS ${TLRENDER_NDI_SDK}/lib/x86_64-linux-gnu/*.so* )
	install( FILES ${HDR_LIB_DSOS}
	    DESTINATION lib
	    COMPONENT applications )
    endif()
endif()

    
if ( APPLE )
    # Install all .dylibs in CMAKE_INSTALL_PREFIX / lib
    file( GLOB HDR_DYLIBS
	${CMAKE_INSTALL_PREFIX}/lib/*.dylib*
    )
    install( FILES ${HDR_DYLIBS}
	DESTINATION lib
	COMPONENT applications )

    # Install all .frameworks in CMAKE_INSTALL_PREFIX / lib
    file( GLOB HDR_FRAMEWORKS
	${CMAKE_INSTALL_PREFIX}/lib/*.framework* )

    foreach( framework ${HDR_FRAMEWORKS} )
	install( DIRECTORY ${framework}
	    DESTINATION lib
	    COMPONENT applications )
    endforeach()
    
elseif ( UNIX )
    # Install all .so in CMAKE_INSTALL_PREFIX/lib
    file( GLOB HDR_LIB_DSOS "${CMAKE_INSTALL_PREFIX}/lib/*.so*" )
    install( FILES ${HDR_LIB_DSOS}
	DESTINATION lib
	COMPONENT applications )
  
    # For RedHat and Rocky Linux, we must install also lib64
    file( GLOB HDR_LIB64_DSOS "${CMAKE_INSTALL_PREFIX}/lib64/*.so*" )
    install( FILES ${HDR_LIB64_DSOS}
      DESTINATION lib64
      COMPONENT applications )
  
elseif(WIN32)
    # For Python, Poco, USD, FLTK, NDI, MSVC runtimes and FFmpeg
    file(GLOB HDR_DLLS "${CMAKE_INSTALL_PREFIX}/bin/*.dll"  )
    
    install(FILES ${HDR_DLLS}
    	DESTINATION bin/
	COMPONENT applications )
endif()


if ( NOT WIN32 )
    install( FILES
	${CMAKE_SOURCE_DIR}/bin/hdr.sh
	DESTINATION bin
        PERMISSIONS 
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
	COMPONENT applications )
endif()

if (APPLE)
    # Define variables for bundle directories in the build dir
    set(HDR_BUNDLE_DIR ${CMAKE_BINARY_DIR}/hdr.app)

    # Create the mrv2.app bundle structure
    file(MAKE_DIRECTORY ${HDR_BUNDLE_DIR}/Contents/MacOS)
    file(MAKE_DIRECTORY ${HDR_BUNDLE_DIR}/Contents/Resources)
endif()
