# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    App.h
    mrvDevicesModel.h
    mrvFilesModel.h
    mrvOpenSeparateAudioDialog.h
    mrvSettingsObject.h
  )
  set(SOURCES
    App.cpp
    mrvDevicesModel.cpp
    mrvFilesModel.cpp
    mrvOpenSeparateAudioDialog.cpp
    mrvSettingsObject.cpp
  )

add_library(mrvApp ${SOURCES} ${HEADERS})
target_link_libraries(mrvApp tlApp mrvGL mrvFl )
target_include_directories(mrvApp PUBLIC ${INCLUDES} )
set_target_properties(mrvApp PROPERTIES FOLDER lib)
set_target_properties(mrvApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvApp)
