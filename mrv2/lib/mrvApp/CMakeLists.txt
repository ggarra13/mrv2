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

set( GL_LIBRARY )
if (TLRENDER_GL)
    set( GL_LIBRARY mrvGL )
endif()

if(WIN32 AND FLTK_BUILD_SHARED_LIBS)
    target_compile_definitions(mrvApp PUBLIC FL_DLL)
endif()

target_link_libraries(mrvApp PUBLIC mrvPanels mrvNetwork ${GL_LIBRARY} mrvDraw mrvFLU mrvWidgets ${FLTK_LIBRARIES} )
target_include_directories(mrvApp PUBLIC ${INCLUDES} )
set_target_properties(mrvApp PROPERTIES FOLDER lib)



install(TARGETS mrvApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries )
