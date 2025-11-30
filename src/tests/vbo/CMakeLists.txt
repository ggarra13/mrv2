# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


set(HEADERS
)
set(SOURCES
    vbo.cpp)

find_package(tlRender REQUIRED)
set(LIBRARIES tlRender)

list(APPEND LIBRARIES ${FLTK_vk_LIBRARY} ${FLTK_LIBRARIES})

if( APPLE )
    set(OSX_FRAMEWORKS "-framework IOKit")
    list(APPEND LIBRARIES ${OSX_FRAMEWORKS})
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;/usr/local/lib")
endif()

add_executable(vbo WIN32 ${SOURCES} ${HEADERS})

target_include_directories( vbo BEFORE PRIVATE . )

target_link_libraries(vbo PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories(vbo BEFORE PUBLIC ${CMAKE_INSTALL_PREFIX}/lib /usr/local/lib )

install(TARGETS vbo
    RUNTIME DESTINATION bin/tests COMPONENT tests
    LIBRARY DESTINATION lib COMPONENT libraries
    ARCHIVE DESTINATION lib COMPONENT libraries )

#
# Handle install
#
# include(InstallRequiredSystemLibraries)


