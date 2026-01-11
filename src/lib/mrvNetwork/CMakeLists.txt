# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvCypher.h
    mrvDummyClient.h
    mrvFilesModelItem.h
    mrvFilePath.h
    mrvTCP.h
    mrvMessage.h
)

set(SOURCES
    mrvCypher.cpp
    mrvFilesModelItem.cpp
    mrvFilePath.cpp
    mrvTCP.cpp
)

set( LIBRARIES mrvFl)

if(MRV2_NETWORK)
    list(APPEND SOURCES
    		mrvClient.cpp
		mrvComfyUIListener.cpp
		mrvImageListener.cpp
    		mrvMessagePublisher.cpp
    		mrvParseHost.cpp
    		mrvSyncClient.cpp
    		mrvSyncMedia.cpp
    		mrvSyncUI.cpp
    		mrvCommandInterpreter.cpp
    		mrvConnectionHandler.cpp
    		mrvServer.cpp )
    list(APPEND HEADERS
              	mrvComfyUIListener.h 
		mrvImageListener.h
    		mrvMessagePublisher.h
		mrvParseHost.h
    		mrvProtocolVersion.h
    		mrvClient.h
    		mrvCommandInterpreter.h
		mrvConnectionHandler.h
    		mrvServer.h)
    list(APPEND LIBRARIES Poco::Net Poco::Foundation )
endif()


add_library(mrvNetwork ${SOURCES} ${HEADERS})
files_to_absolute_paths()

target_link_libraries(mrvNetwork PUBLIC ${LIBRARIES} )
set_target_properties(mrvNetwork PROPERTIES FOLDER lib)

install(TARGETS mrvNetwork
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)
