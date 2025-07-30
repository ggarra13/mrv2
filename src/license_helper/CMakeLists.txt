# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


set(HEADERS
    mrvOS.h
)
set(SOURCES
    mrvOS.cpp
    main.cpp)

list(APPEND LIBRARIES ${FLTK_LIBRARIES})

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
