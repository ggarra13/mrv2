# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


set(HEADERS
    mrvEnv.h
    mrvFile.h
    mrvHome.h
    mrvI8N.h
    mrvLanguages.h
    mrvMainWindow.h
    mrvOS.h
    mrvRoot.h
    mrvString.h
)
set(SOURCES
    main.cpp
    mrvEnv.cpp
    mrvFile.cpp
    mrvHome.cpp
    mrvLanguages.cpp
    mrvOS.cpp
    mrvRoot.cpp
    mrvString.cpp
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
list(APPEND LIBRARIES fltk::fltk ${Intl_LIBRARIES}) # Use static FLTK, not shared one

add_executable(license_helper WIN32 ${SOURCES} ${HEADERS})

target_include_directories(license_helper BEFORE PRIVATE . )

target_link_libraries(license_helper PUBLIC ${LIBRARIES}
    PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories(license_helper
    BEFORE PUBLIC ${CMAKE_INSTALL_PREFIX}/lib )

install(TARGETS license_helper
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION lib COMPONENT libraries
    ARCHIVE DESTINATION lib COMPONENT libraries )
