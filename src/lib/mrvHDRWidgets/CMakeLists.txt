# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


set(FL_SOURCES
    mrvHDRView.fl)

FLTK_WRAP_UI( mrvHDRWidgets ${FL_SOURCES} )

set(HEADERS
    ${mrvHDRWidgets_FLTK_UI_SRCS}
)
set(SOURCES
    ${mrvHDRWidgets_FLTK_UI_SRCS})

hdr_files_to_absolute_paths()

find_package(Intl REQUIRED)


set(LIBRARIES ${Intl_LIBRARIES})

if (WIN32)
    list(APPEND LIBRARIES ${FLTK_LIBRARIES})
else()
    if(FLTK_BUILD_SHARED_LIBS)
	list(APPEND LIBRARIES ${FLTK_LIBRARIES} fltk::vk-shared)
    else()
	list(APPEND LIBRARIES ${FLTK_LIBRARIES} fltk::vk)
    endif()
endif()

if( APPLE )
    set(OSX_FRAMEWORKS "-framework IOKit")
    list(APPEND LIBRARIES ${OSX_FRAMEWORKS})
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;/usr/local/lib")
endif()

# if (UNIX AND NOT APPLE)
#     if (TLRENDER_X11)
# 	list(APPEND LIBRARIES Xss )
#     endif()
# endif()


# if (APPLE)
#     list(PREPEND SOURCES mrvMainWindow.mm)
# endif()

add_library(mrvHDRWidgets ${SOURCES} ${HEADERS})



target_link_libraries(mrvHDRWidgets PUBLIC ${LIBRARIES} )
set_target_properties(mrvHDRWidgets PROPERTIES FOLDER lib)

install(TARGETS mrvHDRWidgets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)
