# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvCypher.h
    mrvCompareOptions.h
    mrvDisplayOptions.h
    mrvDummyClient.h
    mrvFilesModelItem.h
    mrvFilePath.h
    mrvImageOptions.h
    mrvInsertData.h
    mrvLUTOptions.h
    mrvTCP.h
    mrvMessage.h
)

set(SOURCES
    mrvCypher.cpp
    mrvCompareOptions.cpp
    mrvDisplayOptions.cpp
    mrvFilesModelItem.cpp
    mrvFilePath.cpp
    mrvImageOptions.cpp
    mrvInsertData.cpp
    mrvLUTOptions.cpp
    mrvTCP.cpp
)


set( LIBRARIES mrvCore)
if(MRV2_NETWORK)
    list(APPEND SOURCES
    		mrvClient.cpp
    		mrvMessagePublisher.cpp
    		mrvParseHost.cpp
    		mrvSyncClient.cpp
    		mrvSyncMedia.cpp
    		mrvSyncUI.cpp
    		mrvCommandInterpreter.cpp
    		mrvConnectionHandler.cpp
    		mrvServer.cpp )
    list(APPEND HEADERS
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
set_target_properties(mrvNetwork PROPERTIES PUBLIC_HEADER "${HEADERS}")
add_dependencies( mrvNetwork mrvFl )

install(TARGETS mrvNetwork
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv2/mrvNetwork
    COMPONENT libraries)
