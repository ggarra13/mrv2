
include(InstallRequiredSystemLibraries)

set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/mrViewer/docs/LICENSE.txt" )
set( CPACK_PACKAGE_VERSION_MAJOR "${mrViewer2_VERSION_MAJOR}" )
set( CPACK_PACKAGE_VERSION_MINOR "${mrViewer2_VERSION_MINOR}" )
set( CPACK_PACKAGE_VERSION_PATCH "${mrViewer2_VERSION_PATCH}" )

set( CPACK_PACKAGE_NAME mrViewer2 )
set( CPACK_PACKAGE_EXECUTABLES mrViewer mrViewer )
set( CPACK_SOURCE_IGNORE_FILES "src" "include" "lib/*.a" "lib/*.lib" )

if( APPLE )
  set(CPACK_GENERATOR Bundle )

  set( INSTALL_NAME ${PROJECT_NAME} )
  set( ROOT_DIR ${CMAKE_SOURCE_DIR}/mrViewer )

  configure_file(
  ${ROOT_DIR}/etc/macOS/startup.sh.in
  ${PROJECT_BINARY_DIR}/startup.sh)

  configure_file(
  ${ROOT_DIR}/etc/macOS/Info.plist.in
  ${PROJECT_BINARY_DIR}/Info.plist )

  set(CPACK_PACKAGE_ICON ${ROOT_DIR}/etc/macOS/mrViewer.icns )
  set(CPACK_BUNDLE_NAME ${INSTALL_NAME} )
  set(CPACK_BUNDLE_ICON ${ROOT_DIR}/etc/macOS/mrViewer.icns )
  set(CPACK_BUNDLE_PLIST ${PROJECT_BINARY_DIR}/Info.plist )
  set(CPACK_BUNDLE_STARTUP_COMMAND ${PROJECT_BINARY_DIR}/startup.sh)
endif()

# include( CPack )
