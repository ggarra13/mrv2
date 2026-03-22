# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvCPU.h
    mrvEnv.h
    mrvFileManager.h
    mrvOS.h 
    mrvRoot.h
    mrvStackTrace.h
)

set(SOURCES
    mrvCPU.cpp
    mrvMemory.cpp
    mrvOS.cpp
    mrvRoot.cpp
    mrvStackTrace.cpp
)

set(LIBRARIES )

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
    # macOS has a great a built-in stack trace and signal handler.
    # No need to use cpptrace.
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

add_library(mrvOS ${SOURCES} ${HEADERS})

files_to_absolute_paths()

target_link_libraries(mrvOS PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvOS PROPERTIES FOLDER lib)


install(TARGETS mrvOS
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrvOS
    COMPONENT libraries )
