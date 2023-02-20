# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvAnnotationsPanel.h
    mrvColorAreaPanel.h
    mrvColorPanel.h
    mrvComparePanel.h
    mrvDevicesPanel.h
    mrvHistogramPanel.h
    mrvImageInfoPanel.h
    mrvEnvironmentMapPanel.h
    mrvLogsPanel.h
    mrvPlaylistPanel.h
    mrvFilesPanel.h
    mrvSettingsPanel.h
    mrvPanelsCallbacks.h
    mrvPanelWidget.h
)

set(SOURCES
    mrvAnnotationsPanel.cpp
    mrvColorAreaPanel.cpp
    mrvColorPanel.cpp
    mrvComparePanel.cpp
    mrvDevicesPanel.cpp
    mrvHistogramPanel.cpp
    mrvImageInfoPanel.cpp
    mrvEnvironmentMapPanel.cpp
    mrvLogsPanel.cpp
    mrvPlaylistPanel.cpp
    mrvFilesPanel.cpp
    mrvSettingsPanel.cpp
    mrvPanelsCallbacks.cpp
    mrvPanelWidget.cpp
    mrvVectorscopePanel.cpp
)


add_library(mrvPanels ${SOURCES} ${HEADERS})
files_to_absolute_paths()

set( LIBRARIES mrvGL mrvFl )

target_link_libraries(mrvPanels PUBLIC ${LIBRARIES} )
set_target_properties(mrvPanels PROPERTIES FOLDER lib)
set_target_properties(mrvPanels PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvPanels
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvPanels
    COMPONENT libraries)
