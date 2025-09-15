# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrv2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Common CPACK options to all generators
#
string(TIMESTAMP THIS_YEAR "%Y")

set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/../LICENSE" )
set( CPACK_PACKAGE_VERSION_MAJOR "${mrv2_VERSION_MAJOR}" )
set( CPACK_PACKAGE_VERSION_MINOR "${mrv2_VERSION_MINOR}" )
set( CPACK_PACKAGE_VERSION_PATCH "${mrv2_VERSION_PATCH}" )
set( CPACK_PACKAGE_VERSION "${mrv2_VERSION}")
set( CPACK_PACKAGE_CONTACT "ggarra13@gmail.com")

set( mrv2_NAME mrv2 )
if (MRV2_BACKEND STREQUAL "VK")
    set( mrv2_NAME vmrv2 )
endif()


#
# Experimental support in CPack for multithreading. 0 uses all cores.
#
if (APPLE)
    set( CPACK_THREADS 1 )
else()
    set( CPACK_THREADS 0 )
endif()

set( MRV2_OS_BITS 32 )
if(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR
	    CMAKE_SYSTEM_PROCESSOR MATCHES "aarch")
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	    set( MRV2_OS_BITS 64 )
            set( MRV2_ARCHITECTURE "arm64")
	else()
            set( MRV2_ARCHITECTURE "arm")
	endif()
    else()
	set( MRV2_OS_BITS 64 )
        set( MRV2_ARCHITECTURE "amd64")
    endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^mips.*")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set( MRV2_OS_BITS 64 )
        set( MRV2_ARCHITECTURE "mips64el")
    else()
        set( MRV2_ARCHITECTURE "mipsel")
    endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^ppc.*")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set( MRV2_ARCHITECTURE "ppc64le")
    else()
        message(FATAL_ERROR "Architecture is not supported")
    endif()
else()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set( MRV2_OS_BITS 64 )
        set( MRV2_ARCHITECTURE "aarch64")
    else()
        set( MRV2_ARCHITECTURE "x86")
    endif()
endif()

set( mrv2ShortName "${mrv2_NAME}-v${mrv2_VERSION}-${CMAKE_SYSTEM_NAME}-${MRV2_OS_BITS}" )
set( CPACK_PACKAGE_NAME ${mrv2_NAME} )
set( CPACK_PACKAGE_VENDOR "Hernán." )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Professional media player.")
set( CPACK_PACKAGE_INSTALL_DIRECTORY ${mrv2ShortName} )
set( CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-v${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${MRV2_ARCHITECTURE} )

#
# This is the mrv2/ subdir
#
set(MRV2_DIR ${CMAKE_SOURCE_DIR})

#
# This is the root of mrv2
#
file(REAL_PATH "${MRV2_DIR}/.." MRV2_ROOT)

set(CPACK_VERBOSE ON)
set(CPACK_FILTER_IGNORE_FILES "/.git/;/build/;/CMakeFiles/;.o;.a;.lib")

#
# \@bug:
# This dummy (empty) install script is needed so variables get passed to
# the CPACK_PRE_BUILD_SCRIPTS.
#
set( CPACK_INSTALL_SCRIPT ${MRV2_ROOT}/cmake/dummy.cmake )

#
# This pre-build script does some cleaning of files in packaging area to
# keep installers smaller.
#
set( CPACK_PRE_BUILD_SCRIPTS ${MRV2_ROOT}/cmake/prepackage.cmake )

if(APPLE)
    ##############################
    # New Method using Dragndrop #
    ##############################

    set(mrv2_NAME mrv2)
    if (MRV2_BACKEND STREQUAL "VK")
	set(mrv2_NAME vmrv2)
    endif()
    
    set( INSTALL_NAME ${mrv2_NAME} )
    set( HDR_INSTALL_NAME hdr )

    # Define variables for bundle directories in the build dir
    set(MRV2_BUNDLE_DIR ${CMAKE_BINARY_DIR}/${mrv2_NAME}.app)

    # Create the mrv2.app bundle structure
    message(STATUS "CREATE ${mrv2_NAME}.app dir")
    file(MAKE_DIRECTORY ${MRV2_BUNDLE_DIR}/Contents/MacOS)
    file(MAKE_DIRECTORY ${MRV2_BUNDLE_DIR}/Contents/Resources)
    
    # Copy the icon
    configure_file(${MRV2_DIR}/etc/macOS/mrv2.icns
	${MRV2_BUNDLE_DIR}/Contents/Resources/${mrv2_NAME}.icns COPYONLY)
    
    # Copy the shell scripts into the bundles and make them executable
    if (MRV2_BACKEND STREQUAL "VK")
	configure_file(${MRV2_DIR}/etc/macOS/mrv2.sh
	    ${MRV2_BUNDLE_DIR}/Contents/MacOS/${mrv2_NAME}.sh COPYONLY)
    else()
	configure_file(${MRV2_DIR}/etc/macOS/mrv2.sh
	    ${MRV2_BUNDLE_DIR}/Contents/MacOS/${mrv2_NAME} COPYONLY)
    endif()
    
    configure_file(
     	${MRV2_DIR}/etc/macOS/mrv2.plist.in
     	${MRV2_BUNDLE_DIR}/Contents/Info.plist )

    
    install(DIRECTORY ${MRV2_BUNDLE_DIR}
	DESTINATION .
	USE_SOURCE_PERMISSIONS
	COMPONENT applications
    )
    

    
    if (EXISTS ${CMAKE_INSTALL_PREFIX}/bin/hdr)
	
	set(HDR_BUNDLE_DIR ${CMAKE_BINARY_DIR}/hdr.app)
	
	# Create the hdr.app bundle structure
	file(MAKE_DIRECTORY ${HDR_BUNDLE_DIR}/Contents/MacOS)
	file(MAKE_DIRECTORY ${HDR_BUNDLE_DIR}/Contents/Resources)

	# Copy the icon
	message(STATUS "CREATE hdr.app dir")
	file(COPY ${MRV2_DIR}/etc/macOS/hdr.icns
	    DESTINATION ${HDR_BUNDLE_DIR}/Contents/Resources)
    
	# Copy the shell scripts into the bundles and make them executable
	configure_file(${MRV2_DIR}/etc/macOS/hdr.sh ${HDR_BUNDLE_DIR}/Contents/MacOS/hdr COPYONLY)
    
	configure_file(
     	    ${MRV2_DIR}/etc/macOS/hdr.plist.in
     	    ${HDR_BUNDLE_DIR}/Contents/Info.plist )
	
	install(DIRECTORY ${HDR_BUNDLE_DIR}
	    DESTINATION .
	    USE_SOURCE_PERMISSIONS
	    COMPONENT applications
	)
    endif()

    # Configure CPack for DragNDrop
    set(CPACK_GENERATOR "DragNDrop")

    # Package settings
    set(CPACK_PACKAGE_ICON ${MRV2_DIR}/etc/macOS/mrv2.icns )

    # DragNDrop settings
    set(CPACK_DMG_VOLUME_NAME "${mrv2_NAME} Installer")
    set(CPACK_DMG_FORMAT "UDZO")
    Set(CPACK_DMG_VOLUME_NAME ${CPACK_PACKAGE_FILE_NAME})
    set(CPACK_DMG_VOLUME_ICON ${MRV2_DIR}/etc/macOS/mrv2.icns)
    
    
    
    set(CPACK_COMPONENTS_ALL "macos_bundles")
    set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${mrv2_NAME};applications;/")
    set(CPACK_INSTALLED_DIRECTORIES "${CMAKE_BINARY_DIR}/install;.")

elseif(UNIX)
    
    #
    # Linux generators
    #
    set(CPACK_GENERATOR DEB RPM TGZ)
    
    #
    # Linux icon and .desktop shortcut
    #

    #
    # This desktop is the one placed on the desktop for X11/Wayland version
    # shortcuts.
    #
    configure_file( ${MRV2_DIR}/etc/Linux/mrv2.desktop.in
	"${PROJECT_BINARY_DIR}/etc/${mrv2_NAME}-v${mrv2_VERSION}.desktop" )
    configure_file( ${MRV2_DIR}/etc/Linux/hdr.desktop.in
	"${PROJECT_BINARY_DIR}/etc/hdr-v${mrv2_VERSION}.desktop" )

    #
    # This desktop file is for Wayland to set its icon correctly.
    #
    configure_file( ${MRV2_DIR}/etc/Linux/mrv2.main.desktop.in
	"${PROJECT_BINARY_DIR}/etc/${mrv2_NAME}.desktop" )
    install(FILES "${PROJECT_BINARY_DIR}/etc/${mrv2_NAME}-v${mrv2_VERSION}.desktop"
	DESTINATION share/applications COMPONENT applications)
    install(FILES "${PROJECT_BINARY_DIR}/etc/${mrv2_NAME}.desktop"
	DESTINATION share/applications COMPONENT applications)
    install(DIRECTORY ${MRV2_DIR}/share/icons
	DESTINATION share/ COMPONENT applications)


    if (EXISTS "${CMAKE_INSTALL_PREFIX}/bin/hdr")
	configure_file( ${MRV2_DIR}/etc/Linux/hdr.main.desktop.in
	    "${PROJECT_BINARY_DIR}/etc/hdr.desktop")
	configure_file( ${MRV2_DIR}/etc/Linux/hdr.desktop.in
	    "${PROJECT_BINARY_DIR}/etc/hdr-v${mrv2_VERSION}.desktop")
    
	install(FILES "${PROJECT_BINARY_DIR}/etc/hdr.desktop"
	    DESTINATION share/applications COMPONENT applications)
	install(FILES "${PROJECT_BINARY_DIR}/etc/hdr-v${mrv2_VERSION}.desktop"
	    DESTINATION share/applications COMPONENT applications)
    endif()


    set(CPACK_INSTALL_PREFIX /usr/local/${mrv2ShortName})

    #
    # Linux post-install and post-remove scripts to handle versioning and
    # installation of .desktop shortcut on the user's Desktop.
    #
    configure_file(
	${MRV2_DIR}/etc/Linux/postinst.in
	${PROJECT_BINARY_DIR}/etc/Linux/postinst
	@ONLY)
    configure_file(
	${MRV2_DIR}/etc/Linux/postrm.in
	${PROJECT_BINARY_DIR}/etc/Linux/postrm
	@ONLY)

    #
    # set Debian options.
    #
    execute_process(
	COMMAND dpkg --print-architecture
	OUTPUT_VARIABLE DEB_ARCHITECTURE
	OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(CPACK_DEBIAN_PACKAGE_NAME ${mrv2_NAME}-v${mrv2_VERSION})
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${DEB_ARCHITECTURE})
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
	"${PROJECT_BINARY_DIR}/etc/Linux/postinst"
	"${PROJECT_BINARY_DIR}/etc/Linux/postrm")

    set(CPACK_DEBIAN_FILE_NAME	"${CPACK_PACKAGE_FILE_NAME}.deb" )

    #
    # Set RPM options.
    #
    set(CPACK_RPM_PACKAGE_NAME ${mrv2_NAME}-${mrv2_VERSION})

    set(CPACK_RPM_PACKAGE_RELOCATABLE true)
    set(CPACK_RPM_PACKAGE_AUTOREQ false)
    set(CPACK_RPM_PACKAGE_AUTOPROV true)
    set(CPACK_RPM_COMPRESSION_TYPE gzip )
    execute_process(
	COMMAND uname -m
	OUTPUT_VARIABLE RPM_ARCHITECTURE
	OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set( CPACK_RPM_PACKAGE_ARCHITECTURE ${RPM_ARCHITECTURE} )

    set(
	CPACK_RPM_POST_INSTALL_SCRIPT_FILE
	${PROJECT_BINARY_DIR}/etc/Linux/postinst)
    set(
	CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE
	${PROJECT_BINARY_DIR}/etc/Linux/postrm)
      
     # Undocumented option used to avoid .build-id libs listing
     set(CPACK_RPM_SPEC_MORE_DEFINE "%define _build_id_links none")

     # \@bug: According to docs it is not needed, but
     #        RPM packaging won't work properly without it.   
     set(CPACK_SET_DESTDIR true) 
else()

    # Create debug directory for .pdb files
    if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR
	    CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	list(PREPEND CPACK_PRE_BUILD_SCRIPTS ${MRV2_ROOT}/cmake/copy_pdbs.cmake )
    endif()


    if (MRV2_HDR)
	install(FILES
	    C:/Windows/System32/vulkan-1.dll
	    DESTINATION bin
	    COMPONENT applications)
    endif()
    
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${mrv2_NAME}-v${mrv2_VERSION}" )
    
    # There is a bug in NSIS that does not handle full unix paths properly. Make
    # sure there is at least one set of four (4) backlasshes.
    #
    # This removes the page for the user to create
    # Desktop shortcuts, BESIDES modifying the PATH environment variable.
    #
    set(CPACK_NSIS_MODIFY_PATH OFF)
    
    set(CPACK_GENERATOR NSIS ZIP)

    #
    # This sets the title at the top of the installer.
    #
    set(CPACK_NSIS_PACKAGE_NAME "${mrv2_NAME} v${mrv2_VERSION} ${CMAKE_SYSTEM_NAME}-${MRV2_OS_BITS}" )
    
    #
    # Set the executable
    #
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin/mrv2.exe")

    #
    # Set the MUI Installer icon
    #
    set(CPACK_NSIS_MUI_ICON "${MRV2_DIR}/main/app.ico")
    set(CPACK_NSIS_MUI_UNICON "${MRV2_DIR}/main/app.ico")

    #
    # Set the MUI banner to use a custom mrv2 one.
    #
    set(MUI_HEADERIMAGE "${MRV2_ROOT}/cmake/nsis/NSIS_background.bmp")
    file(TO_NATIVE_PATH "${MUI_HEADERIMAGE}" MUI_HEADERIMAGE)
    string(REPLACE "\\" "\\\\" MUI_HEADERIMAGE "${MUI_HEADERIMAGE}")
    set(CPACK_NSIS_MUI_HEADERIMAGE "${MUI_HEADERIMAGE}")

    #
    # Default location for installation.
    #
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")

    #
    # This sets the name in Windows Apps and Control Panel.
    #
    set(mrv2_DISPLAY_NAME "${mrv2_NAME}-${MRV2_OS_BITS} v${mrv2_VERSION}")
    set(hdr_DISPLAY_NAME "hdr-${MRV2_OS_BITS} v${mrv2_VERSION}")
    
    set(CPACK_NSIS_DISPLAY_NAME "${mrv2_DISPLAY_NAME}" )

    set(CPACK_PACKAGE_EXECUTABLES "mrv2" "${mrv2_DISPLAY_NAME}")
    set(CPACK_CREATE_DESKTOP_LINKS "mrv2" "${mrv2_DISPLAY_NAME}")


    #
    # Always create a desktop shortcuts to mrv2.exe and hdr.exe
    #
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
	"CreateShortCut \\\"$DESKTOP\\\\${mrv2_DISPLAY_NAME}.lnk\\\" \\\"$INSTDIR\\\\bin\\\\mrv2.exe\\\"
         CreateShortCut \\\"$DESKTOP\\\\${hdr_DISPLAY_NAME}.lnk\\\" \\\"$INSTDIR\\\\bin\\\\hdr.exe\\\"")

    #
    # Remove the shortcuts on uninstall
    #
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
	"Delete \\\"$DESKTOP\\\\${mrv2_DISPLAY_NAME}.lnk\\\"
	 Delete \\\"$DESKTOP\\\\${hdr_DISPLAY_NAME}.lnk\\\"")
    
    #
    # To call uninstall first if the same version has been installed.
    #

    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON )

    #
    # Create NSIS registry
    #
    configure_file(
	"${MRV2_ROOT}/cmake/nsis/NSISRegistry.cmake.in"
	"${PROJECT_BINARY_DIR}/cmake/nsis/NSISRegistry.cmake"
	@ONLY)
    
    include("${PROJECT_BINARY_DIR}/cmake/nsis/NSISRegistry.cmake")

endif()

#
# For Windows installer, handle the components
#
set(mrv2_COMPONENTS
    applications
    documentation
)

if(BUILD_PYTHON)
    list(APPEND mrv2_COMPONENTS 
	python_demos
	python_tk
    )
endif()

set(CPACK_COMPONENTS_ALL ${mrv2_COMPONENTS})
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "${mrv2_NAME} Application")
set(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "${mrv2_NAME} Documentation")
if(BUILD_PYTHON)
    set(CPACK_COMPONENT_PYTHON_DEMOS_DISPLAY_NAME "${mrv2_NAME} Python Demos")
    set(CPACK_COMPONENT_PYTHON_TK_DISPLAY_NAME "Python TK Libraries")
    set(CPACK_COMPONENT_PYTHON_TK_DISABLED TRUE)
endif()


include(CPack)
