# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


set(HEADERS )
set(SOURCES main.cpp )


list(APPEND LIBRARIES fltk::fltk)

add_executable(launcher WIN32  ${SOURCES} ${HEADERS})

target_link_libraries(launcher PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories(launcher BEFORE PUBLIC ${CMAKE_INSTALL_PREFIX}/lib /usr/local/lib )

install(TARGETS launcher
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION lib COMPONENT libraries
    ARCHIVE DESTINATION lib COMPONENT libraries )
