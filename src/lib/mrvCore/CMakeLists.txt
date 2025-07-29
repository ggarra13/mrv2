# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvActionMode.h
    mrvBackend.h
    mrvColor.h
    mrvColorAreaInfo.h
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
    mrvOS.h 
    mrvPathMapping.h
    mrvRoot.h
    mrvSequence.h
    mrvSHA256.h
    mrvSignalHandler.h
    mrvStackTrace.h
    mrvString.h
    mrvTimeObject.h
    mrvUtil.h
    mrvWait.h
)

set(SOURCES
    mrvColor.cpp
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
    mrvSHA256.cpp
    mrvStackTrace.cpp
    mrvString.cpp
    mrvWait.cpp
    mrvTimeObject.cpp
    mrvUtil.cpp
)

set(LIBRARIES tlRender::tlIO tlRender::tlCore OpenEXR::OpenEXR Imath::Imath ${FLTK_LIBRARIES} )


if( WIN32 )
    list( APPEND HEADERS
	mrvEnv.h)
    list( APPEND SOURCES
	win32/mrvSignalHandler.cpp
        mrvFileManager.cpp
	mrvEnv.cpp )
    list(APPEND LIBRARIES_PRIVATE cpptrace::cpptrace)
elseif( APPLE )
    # macOS has a great a built-in stack trace and signal handler
    list( APPEND SOURCES
        mrvFileManager.mm
    )
else()
    list(APPEND SOURCES
        mrvFileManager.cpp
	Linux/mrvSignalHandler.cpp)
    list(APPEND LIBRARIES_PRIVATE cpptrace::cpptrace)
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
