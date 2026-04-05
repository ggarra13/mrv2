# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


set(HEADERS
    #USD.h
    #USDPrivate.h
)
set(SOURCES
    #USD.cpp
    #USDRead.cpp
    #USDRender.cpp
    main.cpp)
    
find_package(pxr REQUIRED)
add_definitions(-DTLRENDER_USD)

set(LIBRARIES )
list(APPEND LIBRARIES tlRender::tlCore tf usd usdGeom usdAppUtils hd)
list(APPEND LIBRARIES ${FLTK_gl_LIBRARY} ${FLTK_vk_LIBRARY} ${FLTK_LIBRARIES})

if( APPLE )
    set(OSX_FRAMEWORKS "-framework IOKit")
    list(APPEND LIBRARIES ${OSX_FRAMEWORKS})
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;/usr/local/lib")
endif()

add_executable(usdviewer WIN32 ${SOURCES} ${HEADERS})

target_include_directories( usdviewer BEFORE PRIVATE . )

target_link_libraries(usdviewer PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories( usdviewer BEFORE PUBLIC ${CMAKE_INSTALL_PREFIX}/lib /usr/local/lib )

install(TARGETS usdviewer
    RUNTIME DESTINATION bin/tests COMPONENT tests
    LIBRARY DESTINATION lib COMPONENT libraries
    ARCHIVE DESTINATION lib COMPONENT libraries )



