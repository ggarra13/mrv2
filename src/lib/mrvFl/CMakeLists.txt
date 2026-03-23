# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvContextObject.h
    mrvHotkey.h
    mrvInit.h
    mrvIO.h
    mrvLanguages.h
    mrvLaserFadeData.h
    mrvOCIO.h
    mrvPathMapping.h
    mrvPreferences.h
    mrvSession.h
    mrvStereo3DAux.h
    mrvTimelinePlayer.h
    mrvVersioning.h
)

set(SOURCES
    mrvContextObject.cpp
    mrvHotkey.cpp
    mrvInit.cpp
    mrvIO.cpp
    mrvLanguages.cpp
    mrvOCIO.cpp
    mrvPathMapping.cpp
    mrvPreferences.cpp
    mrvSession.cpp
    mrvStereo3DAux.cpp
    mrvTimelinePlayer.cpp
    mrvUSD.cpp
    mrvVersioning.cpp
)

if(TLRENDER_GL AND MRV2_BACKEND STREQUAL "GL")
    list(APPEND SOURCES	mrvSaveImage.cpp mrvSaveMovie.cpp)
endif()

if(TLRENDER_VK AND MRV2_BACKEND STREQUAL "VK")
    list(APPEND SOURCES	mrvSaveImageVk.cpp mrvSaveMovieVk.cpp)
endif()

add_library(mrvFl ${SOURCES} ${HEADERS})
files_to_absolute_paths()

set(LIBRARIES mrvUI mrvImage mrvOptions tlRender::tlDevice tlRender::tlDraw)

if(MRV2_PDF)
    list(APPEND LIBRARIES mrvPDF )
endif()

if (WIN32)
    list(APPEND LIBRARIES comsuppw)
endif()

message(STATUS "mrvFl LIBRARIES=${LIBRARIES}")

target_link_libraries(mrvFl PUBLIC ${LIBRARIES} )
set_target_properties(mrvFl PROPERTIES FOLDER lib)

install(TARGETS mrvFl
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrvFl
    COMPONENT libraries)
