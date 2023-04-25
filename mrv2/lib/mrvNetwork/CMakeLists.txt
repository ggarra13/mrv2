# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvClient.h
    mrvCommandInterpreter.h
    mrvCompareOptions.h
    mrvConnectionHandler.h
    mrvDisplayOptions.h
    mrvDummyClient.h
    mrvFilesModelItem.h
    mrvFilePath.h
    mrvImageOptions.h
    mrvLUTOptions.h
    mrvMessage.h
    mrvProtocolVersion.h
    mrvServer.h
    mrvMessageBroker.h
    mrvTCP.h
)

set(SOURCES
    mrvClient.cpp
    mrvCommandInterpreter.cpp
    mrvCompareOptions.cpp
    mrvConnectionHandler.cpp
    mrvDisplayOptions.cpp
    mrvFilesModelItem.cpp
    mrvFilePath.cpp
    mrvImageOptions.cpp
    mrvLUTOptions.cpp
    # mrvMessage.cpp
    mrvMessageBroker.cpp
    mrvServer.cpp
    mrvSyncClient.cpp
    mrvSyncMedia.cpp
    mrvTCP.cpp
)


add_library(mrvNetwork ${SOURCES} ${HEADERS})
files_to_absolute_paths()

set( LIBRARIES Poco::Net Poco::Foundation )

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
