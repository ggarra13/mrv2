# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
  mrvActionMode.h
  mrvColorSpaces.h
  mrvCPU.h
  mrvEnv.h
  mrvFile.h
  mrvFileManager.h
  mrvFonts.h
  mrvHome.h
  mrvHotkey.h
  mrvI8N.h
  mrvImage.h
  mrvLocale.h
  mrvMedia.h
  mrvMemory.h
  mrvMesh.h
  mrvOrderedMap.h
  mrvPathMapping.h
  mrvRoot.h
  mrvSequence.h
  mrvSignalHandler.h
  mrvStackTrace.h
  mrvString.h
  mrvTimeObject.h
  mrvUtil.h
  )

set(SOURCES
  mrvColorSpaces.cpp
  mrvCPU.cpp
  mrvFile.cpp
  mrvFonts.cpp
  mrvHome.cpp
  mrvHotkey.cpp
  mrvLocale.cpp
  mrvMedia.cpp
  mrvMemory.cpp
  mrvMesh.cpp
  mrvOS.cpp
  mrvPathMapping.cpp
  mrvRoot.cpp
  #mrvSequence.cpp
  mrvString.cpp
  mrvTimeObject.cpp
  mrvUtil.cpp
  )

set(LIBRARIES tlRender Imath::Imath ${FLTK_LIBRARIES} )


if( WIN32 )
    list( APPEND HEADERS
	mrvEnv.h
	mrvOS.h 
	win32/mrvStackTrace.cpp
	win32/mrvSignalHandler.cpp
        mrvFileManager.cpp
	mrvEnv.cpp )
    list( APPEND LIBRARIES_PRIVATE Dbghelp )
elseif( APPLE )
    # macOS has a built-in stack trace and signal handler
    list( APPEND SOURCES
        mrvFileManager.mm
        )
else()
    list(APPEND SOURCES
        mrvFileManager.cpp
	Linux/mrvStackTrace.cpp 
	Linux/mrvSignalHandler.cpp)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR
	    CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	list(APPEND LIBRARIES_PRIVATE backtrace)
    endif()
endif()

add_library(mrvCore ${SOURCES} ${HEADERS})

files_to_absolute_paths()

target_link_libraries(mrvCore PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvCore PROPERTIES FOLDER lib)


install(TARGETS mrvCore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries )
