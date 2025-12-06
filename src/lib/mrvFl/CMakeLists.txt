# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvCallbacks.h
    mrvColorSchemes.h
    mrvConvertImage.h
    mrvContextObject.h
    mrvFileRequester.h
    mrvHotkey.h
    mrvInit.h
    mrvIO.h
    mrvLanguages.h
    mrvLaserFadeData.h
    mrvOCIO.h
    mrvPathMapping.h
    mrvPreferences.h
    mrvSaveOptions.h
    mrvSave.h
    mrvSession.h
    mrvStereo3DAux.h
    mrvTimelinePlayer.h
    mrvVersioning.h
)

set(SOURCES
    mrvCallbacks.cpp
    mrvColorSchemes.cpp
    mrvContextObject.cpp
    mrvConvertImage.cpp
    mrvFileRequester.cpp
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

set(LIBRARIES mrvEdit mrvUI tlRender::tlDraw)

if(TLRENDER_GL AND MRV2_BACKEND STREQUAL "GL")
    list(APPEND LIBRARIES mrvGL )
endif()

if(TLRENDER_VK AND MRV2_BACKEND STREQUAL "VK")
    list(APPEND LIBRARIES mrvVk Vulkan::Vulkan)
endif()

if(MRV2_PDF)
    list(APPEND LIBRARIES mrvPDF )
endif()

if(MRV2_PYBIND11)
    list(APPEND LIBRARIES pybind11::embed )
    if(UNIX AND NOT APPLE)
	# Needed for python3.10 on Rocky and Ubuntu 20.04
	list( APPEND LIBRARIES "util" )
    endif()
endif()

if (WIN32)
    list(APPEND LIBRARIES comsuppw)
endif()

messages(STATUS "mrvFl LIBRARIES=${LIBRARIES}")

target_link_libraries(mrvFl PUBLIC ${LIBRARIES} )
set_target_properties(mrvFl PROPERTIES FOLDER lib)

install(TARGETS mrvFl
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)
