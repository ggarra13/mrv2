# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


set(HEADERS
    mrvFile.h
    mrvHome.h
    mrvIO.h
    mrvLanguages.h
    mrvMainWindow.h
)
set(SOURCES
    main.cpp
    mrvFile.cpp
    mrvHome.cpp
    mrvIO.cpp
    mrvLanguages.cpp
)

lic_files_to_absolute_paths()

set( PO_LIC_SOURCES ${PO_LIC_SOURCES} PARENT_SCOPE)
set( PO_LIC_ABS_SOURCES ${PO_LIC_ABS_SOURCES} PARENT_SCOPE)

if (APPLE)
    list(APPEND SOURCES mrvMainWindow.mm)
else()
    list(APPEND SOURCES mrvMainWindow.cpp)
endif()

#
# Use static FLTK
#
set(LIBRARIES )
list(APPEND LIBRARIES ${FLTK_LIBRARIES} mrvOS ${Intl_LIBRARIES})

add_executable(license_helper WIN32 ${SOURCES} ${HEADERS})

target_include_directories(license_helper BEFORE PRIVATE . )

target_link_libraries(license_helper PUBLIC ${LIBRARIES}
    PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories(license_helper
    BEFORE PUBLIC ${CMAKE_INSTALL_PREFIX}/lib )

if (APPLE)
    # 1. Look for the unversioned DSO (libintl.dylib). 
    # We provide the standard Homebrew paths as hints so it works seamlessly
    # on both architectures.
    find_library(INTL_UNVERSIONED_LIB NAMES intl
        HINTS
            /opt/homebrew/opt/gettext/lib  # Apple Silicon Homebrew
            /usr/local/opt/gettext/lib     # Intel Homebrew
    )

    if (INTL_UNVERSIONED_LIB)
        # 2. Resolve the symlink to get the absolute path to the versioned dylib
        # E.g., /opt/homebrew/opt/gettext/lib/libintl.8.dylib
        get_filename_component(INTL_REAL_PATH "${INTL_UNVERSIONED_LIB}" REALPATH)

        # 3. Extract just the versioned filename
        # E.g., libintl.8.dylib
        get_filename_component(INTL_REAL_NAME "${INTL_REAL_PATH}" NAME)

        # 4. Run install_name_tool using the dynamically resolved paths
        add_custom_command(TARGET license_helper POST_BUILD
            COMMAND install_name_tool -change 
            "${INTL_REAL_PATH}" 
            "@rpath/${INTL_REAL_NAME}" 
            $<TARGET_FILE:license_helper>
            COMMENT "Fixing libintl path to use @rpath/${INTL_REAL_NAME} for license_helper"
        )
    else()
        message(WARNING "Could not find libintl.dylib. Skipping install_name_tool configuration.")
    endif()
endif()

install(TARGETS license_helper
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION lib COMPONENT libraries
    ARCHIVE DESTINATION lib COMPONENT libraries )
