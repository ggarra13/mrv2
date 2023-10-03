# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
  mrvActionMode.h
  mrvColorSpaces.h
  mrvCPU.h
  mrvEnv.h
  mrvEnvironmentMapOptions.h
  mrvEnvironmentMapOptionsInline.h
  mrvFilesPanelOptions.h
  mrvFilesPanelOptionsInline.h
  mrvHelpers.h
  mrvHome.h
  mrvHotkey.h
  mrvI8N.h
  mrvMedia.h
  mrvMemory.h
  mrvMesh.h
  mrvOrderedMap.h
  mrvPathMapping.h
  mrvRoot.h
  mrvSequence.h
  mrvSignalHandler.h
  mrvStackTrace.h
  mrvStereo3DOptions.h
  mrvStereo3DOptionsInline.h
  mrvString.h
  mrvTimeObject.h
  mrvUtil.h
  )

set(SOURCES
  mrvColorSpaces.cpp
  mrvCPU.cpp
  mrvEnvironmentMapOptions.cpp
  mrvFilesPanelOptions.cpp
  mrvHome.cpp
  mrvHotkey.cpp
  mrvMedia.cpp
  mrvMemory.cpp
  mrvMesh.cpp
  mrvPathMapping.cpp
  mrvRoot.cpp
  mrvSequence.cpp
  mrvStereo3DOptions.cpp
  mrvString.cpp
  mrvTimeObject.cpp
  mrvUtil.cpp
  )

set(LIBRARIES tlRender Imath::Imath fltk )


if( WIN32 )
    list( APPEND HEADERS
	mrvEnv.h
	mrvOS.h 
	win32/mrvStackTrace.cpp
	win32/mrvSignalHandler.cpp
        mrvHelpers.cpp
	mrvEnv.cpp )
    list( APPEND LIBRARIES_PRIVATE Dbghelp )
elseif( APPLE )
    # macOS has a built-in stack trace and signal handler
    list( APPEND SOURCES
        mrvHelpers.mm
        )
    # list( APPEND SOURCES
    # 	macOS/mrvStackTrace.cpp 
    # 	macOS/mrvSignalHandler.cpp)
else()
    list( APPEND SOURCES
        mrvHelpers.cpp
	Linux/mrvStackTrace.cpp 
	Linux/mrvSignalHandler.cpp)
endif()

if(TLRENDER_USD)
    list(APPEND LIBRARIES_PRIVATE tf usd usdGeom usdImagingGL usdAppUtils hd)
endif()

add_library(mrvCore ${SOURCES} ${HEADERS})

files_to_absolute_paths()

target_link_libraries(mrvCore PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvCore PROPERTIES FOLDER lib)
#set_target_properties(mrvCore PROPERTIES PUBLIC_HEADER "${HEADERS}")



install(TARGETS mrvCore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv2/mrvCore
    COMPONENT libraries )
