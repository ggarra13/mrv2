# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvAsk.h
    mrvCallbacks.h
    mrvColorSchemes.h
    mrvContextObject.h
    mrvFileRequester.h
    mrvHotkey.h
    mrvIO.h
    mrvLanguages.h
    mrvMenus.h
    mrvPreferences.h
    mrvSaving.h
    mrvTimelinePlayer.h
    mrvUtil.h
    mrvVersioning.h
)

set(SOURCES
    mrvAsk.cpp
    mrvCallbacks.cpp
    mrvColorSchemes.cpp
    mrvContextObject.cpp
    mrvFileRequester.cpp
    mrvHotkey.cpp
    mrvIO.cpp
    mrvLanguages.cpp
    mrvMenus.cpp
    mrvPreferences.cpp
    mrvSaving.cpp
    mrvTimeObject.cpp
    mrvTimelineCreate.cpp
    mrvTimelinePlayer.cpp
    mrvUtil.cpp
    mrvVersioning.cpp
)


add_library(mrvFl ${SOURCES} ${HEADERS})

set( LIBRARIES mrvCore mrvWidgets mrvGL fltk Boost::regex )
if (WIN32)
    list( APPEND LIBRARIES comsuppw )
endif()

target_link_libraries(mrvFl PUBLIC ${LIBRARIES} )
set_target_properties(mrvFl PROPERTIES FOLDER lib)
set_target_properties(mrvFl PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvFl
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvFl
    COMPONENT libraries)
