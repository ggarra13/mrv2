# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")


set(HEADERS )
set(SOURCES main.cpp )

if (WIN32)
   list( APPEND SOURCES ${APP_ICON_RESOURCE_WINDOWS} )
endif()

list( APPEND LIBRARIES mrvApp ${LIBINTL_LIBRARIES} )


if( APPLE )
   set( OSX_FRAMEWORKS "-framework Cocoa -framework IOKit" )
   list( APPEND LIBRARIES ${OSX_FRAMEWORKS} )
endif()

add_executable(mrViewer WIN32 ${SOURCES} ${HEADERS})
target_link_libraries(mrViewer PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories( mrViewer BEFORE PRIVATE ${CMAKE_PREFIX_PATH}/lib )

install(TARGETS mrViewer
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib COMPONENT libraries
  ARCHIVE DESTINATION lib COMPONENT libraries )

include(InstallRequiredSystemLibraries)

if ( NOT WIN32 )
  install( FILES ${CMAKE_SOURCE_DIR}/bin/mrViewer.sh
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
