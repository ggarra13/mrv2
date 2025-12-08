# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvApp.h
    mrvFilesModel.h
    mrvMainControl.h
    mrvOpenSeparateAudioDialog.h
    mrvPlaylistsModel.h
    mrvSettingsObject.h
    mrvStdAnyHelper.h
)
set(SOURCES
    mrvApp.cpp
    mrvFilesModel.cpp
    mrvMainControl.cpp
    mrvOpenSeparateAudioDialog.cpp
    mrvPlaylistsModel.cpp
    mrvSettingsObject.cpp
)

add_library(mrvApp ${SOURCES} ${HEADERS})
files_to_absolute_paths()


set(LIBRARIES mrvBaseApp mrvPanels mrvFLU mrvOptions)
if (TLRENDER_NDI)
    list(APPEND LIBRARIES tlRender::tlDevice)
endif()
list(APPEND LIBRARIES ${FLTK_LIBRARIES})

target_link_libraries(mrvApp PUBLIC ${LIBRARIES} )
target_include_directories(mrvApp PUBLIC ${INCLUDES} )
set_target_properties(mrvApp PROPERTIES FOLDER lib)



install(TARGETS mrvApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries )
