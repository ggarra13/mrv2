include(InstallRequiredSystemLibraries) # for windows runtime

set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE" )
set( CPACK_PACKAGE_VERSION_MAJOR "${mrViewer2_VERSION_MAJOR}" )
set( CPACK_PACKAGE_VERSION_MINOR "${mrViewer2_VERSION_MINOR}" )
set( CPACK_PACKAGE_VERSION_PATCH "${mrViewer2_VERSION_PATCH}" )
set(CPACK_PACKAGE_VERSION "${mrViewer2_VERSION_MAJOR}.${mrViewer2_VERSION_MINOR}.${mrViewer2_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "ggarra13@gmail.com")

set( CPACK_PACKAGE_NAME mrViewer2 )
set( CPACK_PACKAGE_VENDOR "Film Aura, LLC" )
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "mrViewer2 - A professional flipbook and movie player.")
set( CPACK_PACKAGE_EXECUTABLES mrViewer mrViewer )



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
elseif(UNIX)
  set(CPACK_GENERATOR TGZ )
  set(CPACK_SET_DESTDIR true) # Needed
  set(CPACK_INSTALL_PREFIX /usr/local/${mrViewerShortName})
endif()

include( CPack )

cpack_add_component( applications
  DISPLAY_NAME "mrViewer2 application"
  DESCRIPTION "mrViewer2 - A professional flipbook and movie player."
  DEPENDS FFmpeg )
