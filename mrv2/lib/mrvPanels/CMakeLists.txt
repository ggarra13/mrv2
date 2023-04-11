# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvAnnotationsPanel.h
    mrvColorAreaPanel.h
    mrvColorPanel.h
    mrvComparePanel.h
    mrvDevicesPanel.h
    mrvEnvironmentMapPanel.h
    mrvFilesPanel.h
    mrvHistogramPanel.h
    mrvImageInfoPanel.h
    mrvLogsPanel.h
    mrvNetworkPanel.h
    mrvPanelsAux.h
    mrvPanelsCallbacks.h
    mrvPanelWidget.h
    mrvPlaylistPanel.h
    mrvPythonPanel.h
    mrvSettingsPanel.h
    mrvVectorscopePanel.h
)

set(SOURCES
    mrvAnnotationsPanel.cpp
    mrvColorAreaPanel.cpp
    mrvColorPanel.cpp
    mrvComparePanel.cpp
    mrvDevicesPanel.cpp
    mrvFilesPanel.cpp
    mrvHistogramPanel.cpp
    mrvImageInfoPanel.cpp
    mrvEnvironmentMapPanel.cpp
    mrvLogsPanel.cpp
    mrvNetworkPanel.cpp
    mrvPanelWidget.cpp
    mrvPanelsCallbacks.cpp
    mrvPlaylistPanel.cpp
    mrvPythonPanel.cpp
    mrvSettingsPanel.cpp
    mrvVectorscopePanel.cpp
)


add_library(mrvPanels ${SOURCES} ${HEADERS})
files_to_absolute_paths()

set( EXPORT_DYNAMIC )
if ( APPLE )
    set( EXPORT_DYNAMIC -Wl,-export_dynamic )
elseif( UNIX )
    set( EXPORT_DYNAMIC -Wl,--export-dynamic )
endif()

set( LIBRARIES
    mrvFl
    ${EXPORT_DYNAMIC}
    pybind11::embed)

target_link_libraries(mrvPanels PUBLIC ${LIBRARIES} )
set_target_properties(mrvPanels PROPERTIES FOLDER lib)
set_target_properties(mrvPanels PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvPanels
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv2/mrvPanels
    COMPONENT libraries)
