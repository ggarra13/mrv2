# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvCallbacks.h
    mrvColorAreaInfo.h
    mrvColorSchemes.h
    mrvContextObject.h
    mrvFileRequester.h
    mrvHotkey.h
    mrvInit.h
    mrvIO.h
    mrvLanguages.h
    mrvLaserFadeData.h
    mrvPathMapping.h
    mrvPreferences.h
    mrvSaveOptions.h
    mrvSaving.h
    mrvSession.h
    mrvStereo3DAux.h
    mrvTimelinePlayer.h
    mrvVersioning.h
)

set(SOURCES
    mrvCallbacks.cpp
    mrvColorSchemes.cpp
    mrvContextObject.cpp
    mrvFileRequester.cpp
    mrvHotkey.cpp
    mrvInit.cpp
    mrvIO.cpp
    mrvLanguages.cpp
    mrvPathMapping.cpp
    mrvPreferences.cpp
    mrvSaving.cpp
    mrvSession.cpp
    mrvStereo3DAux.cpp
    mrvTimelinePlayer.cpp
    mrvVersioning.cpp
)


add_library(mrvFl ${SOURCES} ${HEADERS})
files_to_absolute_paths()

set( LIBRARIES mrvEdit mrvUI )

if(MRV2_PDF)
    list( APPEND LIBRARIES mrvPDF )
endif()

if(MRV2_PYBIND11)
    list( APPEND LIBRARIES pybind11::embed )
    if(UNIX AND NOT APPLE)
	# Needed for python3.10 on Rocky and Ubuntu 20.04
	list( APPEND LIBRARIES "util" )
    endif()
endif()

if (WIN32)
    list(APPEND LIBRARIES comsuppw)
endif()

target_link_libraries(mrvFl PUBLIC ${LIBRARIES} )
set_target_properties(mrvFl PROPERTIES FOLDER lib)
#set_target_properties(mrvFl PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvFl
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv2/mrvFl
    COMPONENT libraries)
