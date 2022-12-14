# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/../LICENSE.md" )
set( CPACK_PACKAGE_VERSION_MAJOR "${mrViewer2_VERSION_MAJOR}" )
set( CPACK_PACKAGE_VERSION_MINOR "${mrViewer2_VERSION_MINOR}" )
set( CPACK_PACKAGE_VERSION_PATCH "${mrViewer2_VERSION_PATCH}" )
set( CPACK_PACKAGE_VERSION "${mrViewer2_VERSION_MAJOR}.${mrViewer2_VERSION_MINOR}.${mrViewer2_VERSION_PATCH}")
set( SHORTVERSION ${CPACK_PACKAGE_VERSION} )
set( CPACK_PACKAGE_CONTACT "ggarra13@gmail.com")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set( MRV_OS_BITS 64 )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set( MRV_OS_BITS 32 )
endif()

set( mrViewerShortName "mrViewer2-v${SHORTVERSION}-${CMAKE_SYSTEM_NAME}-${MRV_OS_BITS}" )

set( CPACK_PACKAGE_NAME mrViewer2 )
set( CPACK_PACKAGE_VENDOR "Film Aura, LLC" )
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "mrViewer2 - A professional flipbook and movie player.")
set( CPACK_PACKAGE_INSTALL_DIRECTORY ${mrViewerShortName} )

set( ROOT_DIR ${CMAKE_SOURCE_DIR} )

if( APPLE )
    set(CPACK_GENERATOR Bundle )

    set( CPACK_INSTALL_SCRIPTS ${CMAKE_SOURCE_DIR}/../cmake/dummy.cmake )
    set( CPACK_PRE_BUILD_SCRIPTS ${CMAKE_SOURCE_DIR}/../cmake/prepackage.cmake )

    set( INSTALL_NAME ${PROJECT_NAME} )

    configure_file(
	${ROOT_DIR}/etc/macOS/startup.sh.in
	${PROJECT_BINARY_DIR}/startup.sh)

    configure_file(
	${ROOT_DIR}/etc/macOS/Info.plist.in
	${PROJECT_BINARY_DIR}/Info.plist )

    set(CPACK_PACKAGE_ICON ${ROOT_DIR}/etc/macOS/mrViewer2.icns )
    set(CPACK_BUNDLE_NAME ${INSTALL_NAME} )
    set(CPACK_BUNDLE_ICON ${ROOT_DIR}/etc/macOS/mrViewer2.icns )
    set(CPACK_BUNDLE_PLIST ${PROJECT_BINARY_DIR}/Info.plist )
    set(CPACK_BUNDLE_STARTUP_COMMAND ${PROJECT_BINARY_DIR}/startup.sh)
elseif(UNIX)

    set( CPACK_PACKAGING_INSTALL_PREFIX "/usr/local" )

    configure_file( ${ROOT_DIR}/etc/Linux/mrViewer2.desktop.in
	"${PROJECT_BINARY_DIR}/mrViewer2-v${SHORTVERSION}.desktop" )

    install(FILES "${PROJECT_BINARY_DIR}/mrViewer2-v${SHORTVERSION}.desktop"
	DESTINATION share/applications COMPONENT applications)
    install(FILES ${ROOT_DIR}/etc/mrViewer2.png
	DESTINATION share/icons/hicolor/32x32/apps COMPONENT applications)

    set(CPACK_GENERATOR DEB RPM TGZ )

    configure_file(
	${ROOT_DIR}/etc/Linux/postinst.in
	${PROJECT_BINARY_DIR}/etc/Linux/postinst
	@ONLY)
    configure_file(
	${ROOT_DIR}/etc/Linux/postrm.in
	${PROJECT_BINARY_DIR}/etc/Linux/postrm
	@ONLY)

    # Set RPM options.

    set(CPACK_RPM_PACKAGE_NAME ${PROJECT_NAME})
    set(CPACK_RPM_PACKAGE_RELOCATABLE true)
    set(CPACK_RPM_PACKAGE_AUTOREQ false)
    set(CPACK_RPM_PACKAGE_AUTOPROV true)
    set(CPACK_RPM_COMPRESSION_TYPE gzip )

    set(
	CPACK_RPM_POST_INSTALL_SCRIPT_FILE
	${PROJECT_BINARY_DIR}/etc/Linux/postinst)
    set(
	CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE
	${PROJECT_BINARY_DIR}/etc/Linux/postrm)


    # set Debian options.

    set(CPACK_DEBIAN_PACKAGE_NAME ${PROJECT_NAME})

    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
	${PROJECT_BINARY_DIR}/etc/Linux/postinst
	${PROJECT_BINARY_DIR}/etc/Linux/postrm)

    set(CPACK_SET_DESTDIR true) # Needed
    set(CPACK_INSTALL_PREFIX /usr/local/${mrViewerShortName})

else()

    # There is a bug in NSIS that does not handle full unix paths properly. Make
    # sure there is at least one set of four (4) backlasshes.
    set(CPACK_NSIS_MODIFY_PATH ON)

    set(CPACK_GENERATOR ZIP NSIS)

    set(CPACK_NSIS_MUI_ICON ${ROOT_DIR}/icons/viewer.ico)
    set(CPACK_NSIS_MUI_UNICON ${ROOT_DIR}/icons/viewer.ico)

    if (MRV_OS_BITS EQUAL 32)
	set( CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES" )
    else()
	set( CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64" )
    endif()

    set( CPACK_NSIS_DISPLAY_NAME "mrViewer2-${MRV_OS_BITS} v${SHORTVERSION}" )
    set( CPACK_NSIS_PACKAGE_NAME "mrViewer2" )
    set( CPACK_PACKAGE_VENDOR "FilmAura" )
    set( CPACK_PACKAGE_INSTALL_DIRECTORY ${mrViewerPackageName})
    set( CPACK_PACKAGE_EXECUTABLES "mrViewer" "mrViewer2-${MRV_OS_BITS} v${SHORTVERSION}" )
    set( CPACK_CREATE_DESKTOP_LINKS "mrViewer" "mrViewer2-${MRV_OS_BITS} v${SHORTVERSION}" )

    set( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON )

endif()

set(CPACK_COMPONENTS_ALL applications )
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "mrViewer2 Application")
include(CPack)
