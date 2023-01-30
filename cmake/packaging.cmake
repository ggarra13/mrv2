# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrv2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/../LICENSE.md" )
set( CPACK_PACKAGE_VERSION_MAJOR "${mrv2_VERSION_MAJOR}" )
set( CPACK_PACKAGE_VERSION_MINOR "${mrv2_VERSION_MINOR}" )
set( CPACK_PACKAGE_VERSION_PATCH "${mrv2_VERSION_PATCH}" )
set( CPACK_PACKAGE_VERSION "${mrv2_VERSION}")
set( CPACK_PACKAGE_CONTACT "ggarra13@gmail.com")

set( CPACK_THREADS 4 )

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set( MRV2_OS_BITS 64 )
    set( MRV2_ARCHITECTURE amd64 )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set( MRV2_OS_BITS 32 )
    set( MRV2_ARCHITECTURE i386 )
endif()

set( mrv2ShortName "mrv2-v${mrv2_VERSION}-${CMAKE_SYSTEM_NAME}-${MRV2_OS_BITS}" )
set( CPACK_PACKAGE_NAME mrv2 )
set( CPACK_PACKAGE_VENDOR "Film Aura, LLC" )
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "mrv2 - A professional flipbook and movie player.")
set( CPACK_PACKAGE_INSTALL_DIRECTORY ${mrv2ShortName} )
set( CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${MRV2_ARCHITECTURE} )

set( ROOT_DIR ${CMAKE_SOURCE_DIR} )

#
# This dummy (empty) install script is needed so variables get passed to
# the CPACK_PRE_BUILD_SCRIPTS. @bug: cmake 3.21 at leaset
set( CPACK_INSTALL_SCRIPT ${CMAKE_SOURCE_DIR}/../cmake/dummy.cmake )
set( CPACK_PRE_BUILD_SCRIPTS ${CMAKE_SOURCE_DIR}/../cmake/prepackage.cmake )

if( APPLE )
    set(CPACK_GENERATOR Bundle )

    set( INSTALL_NAME ${PROJECT_NAME} )

    configure_file(
	${ROOT_DIR}/etc/macOS/startup.sh.in
	${PROJECT_BINARY_DIR}/startup.sh)

    configure_file(
	${ROOT_DIR}/etc/macOS/Info.plist.in
	${PROJECT_BINARY_DIR}/Info.plist )

    set(CPACK_PACKAGE_ICON ${ROOT_DIR}/etc/macOS/mrv2.icns )
    set(CPACK_BUNDLE_NAME ${INSTALL_NAME} )
    set(CPACK_BUNDLE_ICON ${ROOT_DIR}/etc/macOS/mrv2.icns )
    set(CPACK_BUNDLE_PLIST ${PROJECT_BINARY_DIR}/Info.plist )
    set(CPACK_BUNDLE_STARTUP_COMMAND ${PROJECT_BINARY_DIR}/startup.sh)
elseif(UNIX)


    configure_file( ${ROOT_DIR}/etc/Linux/mrv2.desktop.in
	"${PROJECT_BINARY_DIR}/etc/mrv2-v${mrv2_VERSION}.desktop" )

    install(FILES "${PROJECT_BINARY_DIR}/etc/mrv2-v${mrv2_VERSION}.desktop"
	DESTINATION share/applications COMPONENT applications)
    install(FILES ${ROOT_DIR}/etc/mrv2.png
	DESTINATION share/icons/hicolor/32x32/apps COMPONENT applications)

    set(CPACK_GENERATOR DEB RPM TGZ )
    set(CPACK_INSTALL_PREFIX /usr/local/${mrv2ShortName})

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
    if ( "${MRV2_ARCHITECTURE}" STREQUAL "amd64" )
	set( RPM_ARCHITECTURE x86_64 )
    else()
	set( RPM_ARCHITECTURE i386 )
    endif()
    set( CPACK_RPM_PACKAGE_ARCHITECTURE ${RPM_ARCHITECTURE} )

    set(
	CPACK_RPM_POST_INSTALL_SCRIPT_FILE
	${PROJECT_BINARY_DIR}/etc/Linux/postinst)
    set(
	CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE
	${PROJECT_BINARY_DIR}/etc/Linux/postrm)


    #
    # set Debian options.
    #
    set(CPACK_DEBIAN_PACKAGE_NAME ${PROJECT_NAME})
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${MRV2_ARCHITECTURE})
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
	${PROJECT_BINARY_DIR}/etc/Linux/postinst
	${PROJECT_BINARY_DIR}/etc/Linux/postrm)

    set(CPACK_DEBIAN_FILE_NAME
	"${CPACK_DEBIAN_PACKAGE_NAME}-${mrv2_VERSION}-${MRV2_ARCHITECTURE}.deb" )


    set(CPACK_SET_DESTDIR true) # Needed

else()

    # There is a bug in NSIS that does not handle full unix paths properly. Make
    # sure there is at least one set of four (4) backlasshes.
    set(CPACK_NSIS_MODIFY_PATH ON)

    set(CPACK_GENERATOR ZIP NSIS)

    set(CPACK_NSIS_MUI_ICON ${ROOT_DIR}/icons/viewer.ico)
    set(CPACK_NSIS_MUI_UNICON ${ROOT_DIR}/icons/viewer.ico)

    if (MRV2_OS_BITS EQUAL 32)
	set( CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES" )
    else()
	set( CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64" )
    endif()

    set( CPACK_NSIS_DISPLAY_NAME "mrv2-${MRV2_OS_BITS} v${mrv2_VERSION}" )
    set( CPACK_NSIS_PACKAGE_NAME "mrv2" )
    set( CPACK_PACKAGE_VENDOR "FilmAura" )
    set( CPACK_PACKAGE_INSTALL_DIRECTORY ${mrv2PackageName})
    set( CPACK_PACKAGE_EXECUTABLES "mrv2" "mrv2-${MRV2_OS_BITS} v${mrv2_VERSION}" )
    set( CPACK_CREATE_DESKTOP_LINKS "mrv2" "mrv2-${MRV2_OS_BITS} v${mrv2_VERSION}" )

    set( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON )

    include( ${ROOT_DIR}/../cmake/nsis/NSISRegistry.cmake )

endif()

set(CPACK_COMPONENTS_ALL applications )
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "mrv2 Application")
include(CPack)
