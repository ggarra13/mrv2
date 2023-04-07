# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
  mrvColorSpaces.h
  mrvCPU.h
  mrvEnv.h
  mrvHome.h
  mrvHotkey.h
  mrvI8N.h
  mrvMedia.h
  mrvMemory.h
  mrvMesh.h
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
  mrvHome.cpp
  mrvHotkey.cpp
  mrvMedia.cpp
  mrvMemory.cpp
  mrvMesh.cpp
  mrvRoot.cpp
  mrvSequence.cpp
  mrvString.cpp
  mrvTimeObject.cpp
  mrvUtil.cpp
  )

set(LIBRARIES tlTimeline tlDevice tlIO tlCore OCIO OTIO Imath fltk )
set(LIBRARIES_PRIVATE RtAudio FSeq FreeType )
if ( FFmpeg_FOUND )
    list( APPEND LIBRARIES_PRIVATE FFmpeg )
endif()
if ( JPEG_FOUND )
    list( APPEND LIBRARIES_PRIVATE JPEG )
endif()
if ( PNG_FOUND )
    list( APPEND LIBRARIES_PRIVATE PNG )
endif()
if ( TIFF_FOUND )
    list( APPEND LIBRARIES_PRIVATE TIFF )
endif()
if ( OpenEXR_FOUND )
    list( APPEND LIBRARIES_PRIVATE OpenEXR )
endif()

if( WIN32 )
    list( APPEND HEADERS
	mrvEnv.h
	mrvOS.h 
	win32/mrvStackTrace.cpp
	win32/mrvSignalHandler.cpp
	mrvEnv.cpp )
    list( APPEND LIBRARIES_PRIVATE Dbghelp )
elseif( APPLE )
    # macOS has a built-in stack trace and signal handler
    # list( APPEND SOURCES
    # 	macOS/mrvStackTrace.cpp 
    # 	macOS/mrvSignalHandler.cpp)
else()
    list( APPEND SOURCES
	Linux/mrvStackTrace.cpp 
	Linux/mrvSignalHandler.cpp)
endif()


add_library(mrvCore ${SOURCES} ${HEADERS})

files_to_absolute_paths()

target_link_libraries(mrvCore PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvCore PROPERTIES FOLDER lib)
set_target_properties(mrvCore PROPERTIES PUBLIC_HEADER "${HEADERS}")



install(TARGETS mrvCore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv2/mrvCore
    COMPONENT libraries )
