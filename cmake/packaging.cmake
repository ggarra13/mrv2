# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/../LICENSE.md" )
set( CPACK_PACKAGE_VERSION_MAJOR "${mrViewer2_VERSION_MAJOR}" )
set( CPACK_PACKAGE_VERSION_MINOR "${mrViewer2_VERSION_MINOR}" )
set( CPACK_PACKAGE_VERSION_PATCH "${mrViewer2_VERSION_PATCH}" )
set( CPACK_PACKAGE_VERSION "${mrViewer2_VERSION_MAJOR}.${mrViewer2_VERSION_MINOR}.${mrViewer2_VERSION_PATCH}")
set( CPACK_PACKAGE_CONTACT "ggarra13@gmail.com")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set( MRV_OS_BITS 64 )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set( MRV_OS_BITS 32 )
endif()

set( mrViewerShortName "mrViewer2-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${MRV_OS_BITS}" )

set( CPACK_PACKAGE_NAME mrViewer2 )
set( CPACK_PACKAGE_VENDOR "Film Aura, LLC" )
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "mrViewer2 - A professional flipbook and movie player.")
set( CPACK_PACKAGE_EXECUTABLES mrViewer mrViewer )
set( CPACK_PACKAGE_INSTALL_DIRECTORY ${mrViewerShortName} )
set( CPACK_PACKAGING_INSTALL_PREFIX "/usr/local" )

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
  set(CPACK_GENERATOR DEB RPM TGZ )
  set(CPACK_SET_DESTDIR true) # Needed
  set(CPACK_INSTALL_PREFIX /usr/local/${mrViewerShortName})
else()
  set(CPACK_GENERATOR NSIS)
endif()

include(CPack)
