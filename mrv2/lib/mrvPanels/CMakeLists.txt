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
    mrvPanelsAux.h
    mrvPanelsCallbacks.h
    mrvPanelWidget.h
    mrvPlaylistPanel.h
    mrvSettingsPanel.h
    mrvStereo3DPanel.h
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
    mrvPanelWidget.cpp
    mrvPanelsAux.cpp
    mrvPanelsCallbacks.cpp
    mrvPlaylistPanel.cpp
    mrvSettingsPanel.cpp
    mrvStereo3DPanel.cpp
    mrvVectorscopePanel.cpp
)

set( LIBRARIES mrvFl mrvEdit)
	
if(TLRENDER_USD)
    list( APPEND HEADERS mrvUSDPanel.h )
    list( APPEND SOURCES mrvUSDPanel.cpp)
endif()

if(MRV2_PYBIND11)
    list( APPEND LIBRARIES pybind11::embed )
    list( APPEND HEADERS mrvPythonPanel.h )
    list( APPEND SOURCES mrvPythonPanel.cpp )
    if(UNIX AND NOT APPLE)
	# Needed for python3.10 on Rocky and Ubuntu 20.04
	list( APPEND LIBRARIES "util" )
    endif()
endif()

if(MRV2_NETWORK)
    list( APPEND HEADERS mrvNetworkPanel.h )
    list( APPEND SOURCES mrvNetworkPanel.cpp )
endif()

add_library(mrvPanels ${SOURCES} ${HEADERS})
files_to_absolute_paths()



target_link_libraries(mrvPanels PUBLIC ${LIBRARIES} )
set_target_properties(mrvPanels PROPERTIES FOLDER lib)

install(TARGETS mrvPanels
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)
