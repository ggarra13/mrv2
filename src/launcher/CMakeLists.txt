# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


set(HEADERS )
set(SOURCES main.cpp )


list(APPEND LIBRARIES fltk::fltk)

add_executable(vmrv2 WIN32  ${SOURCES} ${HEADERS})

target_link_libraries(vmrv2 PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories(vmrv2 BEFORE PUBLIC ${CMAKE_INSTALL_PREFIX}/lib /usr/local/lib )

install(TARGETS vmrv2
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION lib COMPONENT libraries
    ARCHIVE DESTINATION lib COMPONENT libraries )
