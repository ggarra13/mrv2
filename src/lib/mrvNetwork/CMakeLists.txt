# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvCypher.h
    mrvDummyClient.h
    mrvFilesModelItem.h
    mrvTCP.h
    mrvMessage.h
)

set(SOURCES
    mrvCypher.cpp
    mrvFilesModelItem.cpp
    mrvTCP.cpp
)

set( LIBRARIES mrvFLTK mrvVoice)

if(MRV2_NETWORK)
    list(APPEND SOURCES
	# Common
	mrvSyncClient.cpp
    	mrvSyncMedia.cpp
    	mrvSyncUI.cpp
    	mrvCommandInterpreter.cpp
		
	# TCP

	mrvComfyUIListener.cpp
	mrvImageListener.cpp

	# WebRTC
	mrvFileTransferClient.cpp
	mrvFileTransferServer.cpp
	mrvSignalingClient.cpp
	mrvWebRTCClient.cpp
	mrvWebRTCManager.cpp
    )
    list(APPEND HEADERS
	
	# TCP
        mrvComfyUIListener.h 
	mrvImageListener.h
	
    	mrvProtocolVersion.h
    	mrvCommandInterpreter.h
	
	# WebRTC
	mrvFileChunk.h
	mrvFileTransferClient.h
	mrvFileTransferServer.h
	mrvSignalingClient.h
	mrvSignalingMessage.h
	mrvWebRTCClient.h
	mrvWebRTCManager.h
    )
    list(APPEND LIBRARIES Poco::Net Poco::Foundation)

    find_package(LibDataChannel REQUIRED)
    list(APPEND LIBRARIES LibDataChannel::LibDataChannel)
endif()


add_library(mrvNetwork ${SOURCES} ${HEADERS})
if (NOT EMSCRIPTEN)
    files_to_absolute_paths()
endif()

target_compile_definitions(mrvNetwork
    PRIVATE
    XXH_INLINE_ALL)
target_link_libraries(mrvNetwork PUBLIC ${LIBRARIES} )
set_target_properties(mrvNetwork PROPERTIES FOLDER lib)

install(TARGETS mrvNetwork
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrvNetwork
    COMPONENT libraries)
