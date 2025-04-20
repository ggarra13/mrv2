# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


set(HEADERS
)
set(SOURCES
    fbo.cpp)


set(LIBRARIES tlRender::tlVk tlRender)

list(APPEND LIBRARIES ${FLTK_vk_LIBRARY} ${FLTK_LIBRARIES})

if( APPLE )
    set(OSX_FRAMEWORKS "-framework IOKit")
    list(APPEND LIBRARIES ${OSX_FRAMEWORKS})
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;/usr/local/lib")
endif()

add_executable(fbo WIN32 ${SOURCES} ${HEADERS})

target_include_directories( fbo BEFORE PRIVATE . )

target_link_libraries(fbo PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories( fbo BEFORE PUBLIC ${CMAKE_INSTALL_PREFIX}/lib /usr/local/lib )

install(TARGETS fbo
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION lib COMPONENT libraries
    ARCHIVE DESTINATION lib COMPONENT libraries )

#
# Handle install
#
include(InstallRequiredSystemLibraries)


