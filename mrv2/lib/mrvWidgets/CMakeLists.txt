# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set( FL_SOURCES
    make_ocio_chooser.fl
    keyboard_ui.fl
    mrvAboutUI.fl
    mrvHotkeyUI.fl
    mrvHUDUI.fl
    PathMappingUI.fl
    mrvPreferencesUI.fl
    SaveOptionsUI.fl
    mrViewer.fl
)

FLTK_WRAP_UI( mrvWidgets ${FL_SOURCES} )
#FLTK_RUN_FLUID( mrvWidgets_FLTK_UI_SRCS "${FL_SOURCES}" )

set(HEADERS
    mrvButton.h
    mrvBrowser.h
    mrvChoice.h
    mrvClipButton.h
    mrvCollapsibleGroup.h
    mrvColorInfo.h
    mrvDockGroup.h
    mrvDragButton.h
    mrvDropWindow.h
    mrvFileButton.h
    mrvFileDragger.h
    mrvFunctional.h
    mrvHistogram.h
    mrvHorSlider.h
    mrvLogDisplay.h
    mrvMainWindow.h
    mrvMenuBar.h
    mrvMultilineInput.h
    mrvOCIOBrowser.h
    mrvPack.h
    mrvPathMappingBrowser.h
    mrvPanelButton.h
    mrvPanelGroup.h
    mrvPanelWindow.h
    mrvPlaylistButton.h
    mrvPopupAudio.h
    mrvPopupMenu.h
    mrvPreferencesTree.h
    mrvProgressReport.h
    mrvResizableBar.h
    mrvSecondaryWindow.h
    mrvSlider.h
    mrvStateButton.h
    mrvStatusBar.h
    mrvTable.h
    mrvTile.h
    mrvTextDisplay.h
    mrvTimecode.h
    mrvTimelineGroup.h
    mrvVectorscope.h
    mrvVersion.h
    mrvVolumeSlider.h
)

set(SOURCES
    ${mrvWidgets_FLTK_UI_SRCS}

    mrvBrowser.cpp
    mrvButton.cpp
    mrvChoice.cpp
    mrvCollapsibleGroup.cpp
    mrvColorInfo.cpp
    mrvDockGroup.cpp
    mrvDragButton.cpp
    mrvDropWindow.cpp
    mrvFileButton.cpp
    mrvFileDragger.cpp
    mrvHistogram.cpp
    mrvHorSlider.cpp
    mrvLogDisplay.cpp
    mrvMainWindow.cpp
    mrvMenuBar.cpp
    mrvMultilineInput.cpp
    mrvOCIOBrowser.cpp
    mrvPack.cpp
    mrvPathMappingBrowser.cpp
    mrvPanelButton.cpp
    mrvPanelGroup.cpp
    mrvPanelWindow.cpp
    mrvPlaylistButton.cpp
    mrvPopupAudio.cpp
    mrvPopupMenu.cpp
    mrvPreferencesTree.cpp
    mrvProgressReport.cpp
    mrvResizableBar.cpp
    mrvSecondaryWindow.cpp
    mrvSlider.cpp
    mrvStatusBar.cpp
    mrvTable.cpp
    mrvTextDisplay.cpp
    mrvTimecode.cpp
    mrvTimelineGroup.cpp
    mrvTile.cpp
    mrvVectorscope.cpp
    mrvVersion.cpp
    mrvVolumeSlider.cpp
)


list( APPEND LIBRARIES mrvFlmm mrvFLU mrvCore ${FLTK_LIBRARIES} ${JASPER_LIBRARIES} MINIZIP::minizip )
if(MRV2_PYBIND11)
    list( APPEND LIBRARIES pybind11::embed )
    list( APPEND SOURCES mrvPythonEditor.cpp mrvPythonOutput.cpp)
    list( APPEND HEADERS mrvPythonEditor.h mrvPythonOutput.h)
    
    if(UNIX AND NOT APPLE)
	# Needed for python3.10 on Rocky and Ubuntu 20.04
	list( APPEND LIBRARIES "util" )
    endif()
endif()


files_to_absolute_paths()

if ( APPLE )
    list( PREPEND SOURCES mrvMainWindow.mm )
endif()

add_library(mrvWidgets ${SOURCES} ${HEADERS})


if ( UNIX)
    if ( NOT APPLE )
	add_definitions( -DFLTK_USE_X11 )
	list(APPEND LIBRARIES Xss )
    endif()
endif()

if( WIN32 )
    list(APPEND LIBRARIES Crypt32.lib)
endif()

target_compile_definitions(mrvWidgets PUBLIC FL_DLL)
target_link_libraries(mrvWidgets PUBLIC ${LIBRARIES} )
set_target_properties(mrvWidgets PROPERTIES FOLDER lib)
#set_target_properties(mrvWidgets PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvWidgets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv2/mrvWidgets
    COMPONENT libraries)
