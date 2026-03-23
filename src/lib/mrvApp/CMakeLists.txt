# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvApp.h
    mrvFilesModel.h
    mrvGlobals.h
    mrvMainControl.h
    mrvPlaylistsModel.h
    mrvSettingsObject.h
    mrvStdAnyHelper.h
)
set(SOURCES
    mrvApp.cpp
    mrvFilesModel.cpp
    mrvGlobals.cpp
    mrvMainControl.cpp
    mrvPlaylistsModel.cpp
    mrvSettingsObject.cpp
)

add_library(mrvApp ${SOURCES} ${HEADERS})
files_to_absolute_paths()


set(LIBRARIES mrvBaseApp)
list(APPEND LIBRARIES mrvPanels mrvOptions)

if (TLRENDER_NDI)
    list(APPEND LIBRARIES tlRender::tlDevice)
endif()

target_link_libraries(mrvApp PUBLIC ${LIBRARIES} )
target_include_directories(mrvApp PUBLIC ${INCLUDES} )
set_target_properties(mrvApp PROPERTIES FOLDER lib)



install(TARGETS mrvApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrvApp
    COMPONENT libraries )
