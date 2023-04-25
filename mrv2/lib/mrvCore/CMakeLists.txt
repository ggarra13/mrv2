# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
  mrvColorSpaces.h
  mrvCPU.h
  mrvEnv.h
  mrvEnvironmentMapOptions.h
  mrvEnvironmentMapOptionsInline.h
  mrvHome.h
  mrvHotkey.h
  mrvI8N.h
  mrvMedia.h
  mrvMemory.h
  mrvMesh.h
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
  mrvEnvironmentMapOptions.cpp
  mrvHome.cpp
  mrvHotkey.cpp
  mrvMedia.cpp
  mrvMemory.cpp
  mrvMesh.cpp
  mrvPathMapping.cpp
  mrvRoot.cpp
  mrvSequence.cpp
  mrvString.cpp
  mrvTimeObject.cpp
  mrvUtil.cpp
  )

set(LIBRARIES tlRender tlRender::tlCore Imath::Imath fltk )


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
