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
    mrvImageOps.h
    mrvLicensing.h
    mrvLocale.h
    mrvMedia.h
    mrvMemory.h
    mrvMesh.h
    mrvOrderedMap.h
    mrvOS.h 
    mrvPathMapping.h
    mrvPixelConverter.h
    mrvRoot.h
    mrvSequence.h
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
    mrvImageOps.cpp
    mrvLicensing.cpp
    mrvLocale.cpp
    mrvMedia.cpp
    mrvMemory.cpp
    mrvMesh.cpp
    mrvOS.cpp
    mrvPathMapping.cpp
    mrvRoot.cpp
    mrvStackTrace.cpp
    mrvString.cpp
    mrvTimeObject.cpp
    mrvUtil.cpp
    mrvWait.cpp
)

if (MRV2_NETWORK)
    find_package(Poco REQUIRED Net NetSSL JSON Foundation)
endif()
if (TLRENDER_NET)
    find_package(OpenSSL REQUIRED)
endif()

set(LIBRARIES tlRender::tlIO tlRender::tlCore OpenEXR::OpenEXR Imath::Imath ${FLTK_LIBRARIES})

if (MRV2_NETWORK)
    set(LIBRARIES_PRIVATE
	Poco::Net
	Poco::NetSSL
	Poco::JSON
	Poco::Foundation
    )
endif()
if (TLRENDER_NET)
    list(APPEND LIBRARIES_PRIVATE
	OpenSSL::SSL
	OpenSSL::Crypto)
endif()

if( WIN32 )
    list( APPEND HEADERS
	mrvEnv.h
	mrvFileManager.h)
    list( APPEND SOURCES
	mrvEnv.cpp
	mrvFileManager.cpp
	mrvEnv.cpp)
    if (MRV2_CPPTRACE)
	list(APPEND HEADERS
	    mrvSignalHandler.h)
	list(APPEND SOURCES
	    win32/mrvSignalHandler.cpp)
	list(APPEND LIBRARIES_PRIVATE cpptrace::cpptrace)
    endif()
elseif( APPLE )
    # macOS has a great a built-in stack trace and signal handler
    list( APPEND SOURCES
       mrvFileManager.mm
    )
else()
    list(APPEND SOURCES
        mrvFileManager.cpp
	Linux/mrvSignalHandler.cpp)
    if (MRV2_CPPTRACE)
	list(APPEND LIBRARIES_PRIVATE cpptrace::cpptrace)
    endif()
endif()

add_library(mrvCore ${SOURCES} ${HEADERS})

files_to_absolute_paths()

if (WIN32)
   target_compile_definitions(mrvCore PUBLIC -DFL_DLL)
 endif()
target_link_libraries(mrvCore PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvCore PROPERTIES FOLDER lib)


install(TARGETS mrvCore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries )
