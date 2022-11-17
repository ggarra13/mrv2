set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")


set(HEADERS )
set(SOURCES main.cpp )

if (WIN32)
   list( APPEND SOURCES ${APP_ICON_RESOURCE_WINDOWS} )
endif()

list( APPEND LIBRARIES mrvPlayApp ${LIBINTL_LIBRARIES} )


if( APPLE )
   set( OSX_FRAMEWORKS "-framework Cocoa -framework IOKit" )
   list( APPEND LIBRARIES ${OSX_FRAMEWORKS} )
endif()

add_executable(mrViewer WIN32 ${SOURCES} ${HEADERS})
target_link_libraries(mrViewer PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories( mrViewer BEFORE PRIVATE ${CMAKE_PREFIX_PATH}/lib )

install(TARGETS mrViewer
  RUNTIME DESTINATION bin
  COMPONENT applications)

include(InstallRequiredSystemLibraries)

if ( NOT WIN32 )
   install( FILES ${CMAKE_SOURCE_DIR}/bin/mrViewer.sh
   	    	  ${CMAKE_SOURCE_DIR}/bin/environment.sh
	    DESTINATION bin
            COMPONENT applications )
endif()
