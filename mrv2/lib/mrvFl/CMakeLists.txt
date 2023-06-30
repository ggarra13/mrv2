# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvAsk.h
    mrvCallbacks.h
    mrvColorAreaInfo.h
    mrvColorSchemes.h
    mrvContextObject.h
    mrvEditMode.h
    mrvFileRequester.h
    mrvHotkey.h
    mrvInit.h
    mrvIO.h
    mrvLanguages.h
    mrvMenus.h
    mrvPathMapping.h
    mrvPreferences.h
    mrvSaving.h
    mrvSession.h
    mrvStereo3DAux.h
    mrvTimeObject.h
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
    mrvInit.cpp
    mrvIO.cpp
    mrvLanguages.cpp
    mrvMenus.cpp
    mrvPathMapping.cpp
    mrvPreferences.cpp
    mrvSaving.cpp
    mrvSession.cpp
    mrvStereo3DAux.cpp
    mrvTimelineCreate.cpp
    mrvTimelinePlayer.cpp
    mrvTimeObject.cpp
    mrvUtil.cpp
    mrvVersioning.cpp
)


add_library(mrvFl ${SOURCES} ${HEADERS})
files_to_absolute_paths()

set( LIBRARIES mrvWidgets mrvPDF pybind11::embed )
if (WIN32)
    list(APPEND LIBRARIES comsuppw)
endif()

target_link_libraries(mrvFl PUBLIC ${LIBRARIES} )
set_target_properties(mrvFl PROPERTIES FOLDER lib)
set_target_properties(mrvFl PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvFl
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv2/mrvFl
    COMPONENT libraries)
