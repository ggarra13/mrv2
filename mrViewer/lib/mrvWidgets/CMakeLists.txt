# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

set( FL_SOURCES
  make_ocio_chooser.fl
  keyboard_ui.fl
  mrvAboutUI.fl
  mrvHotkeyUI.fl
  mrvPreferencesUI.fl
  mrViewer.fl
  )

set( USE_FLTK_RUN_FLUID_MACRO 1 )
if ( USE_FLTK_RUN_FLUID_MACRO )
  FLTK_RUN_FLUID( mrvWidgets_FLTK_UI_SRCS "${FL_SOURCES}" )
else()
  FLTK_WRAP_UI( mrvWidgets ${FL_SOURCES} )
endif()

set(HEADERS
  
  mrvButton.h
  mrvBrowser.h
  mrvClipButton.h
  mrvCollapsibleGroup.h
  mrvColorInfo.h
  mrvDockGroup.h
  mrvDragButton.h
  mrvDropWindow.h
  mrvFileButton.h
  mrvFunctional.h
  mrvHistogram.h
  mrvHorSlider.h
  mrvLogDisplay.h
  mrvMainWindow.h
  mrvMultilineInput.h
  mrvOCIOBrowser.h
  mrvPack.h
  mrvPopupMenu.h
  mrvPreferencesTree.h
  mrvProgressReport.h
  mrvResizableBar.h
  mrvSecondaryWindow.h
  mrvSlider.h
  mrvStatusBar.h
  mrvTable.h
  mrvTextDisplay.h
  mrvTimecode.h
  mrvTimelineSlider.h
  mrvToolGroup.h
  mrvToolWindow.h
  mrvVectorscope.h
  mrvVersion.h
  mrvVolumeSlider.h
  )

set(SOURCES
  ${mrvWidgets_FLTK_UI_SRCS}

  mrvBrowser.cpp
  mrvButton.cpp
  mrvCollapsibleGroup.cpp
  mrvColorInfo.cpp
  mrvDockGroup.cpp
  mrvDragButton.cpp
  mrvDropWindow.cpp
  mrvFileButton.cpp
  mrvHistogram.cpp
  mrvHorSlider.cpp
  mrvLogDisplay.cpp
  mrvMainWindow.cpp
  mrvMultilineInput.cpp
  mrvOCIOBrowser.cpp
  mrvPack.cpp
  mrvPopupMenu.cpp
  mrvPreferencesTree.cpp
  mrvProgressReport.cpp
  mrvResizableBar.cpp
  mrvSecondaryWindow.cpp
  mrvSlider.cpp
  mrvTable.cpp
  mrvTextDisplay.cpp
  mrvTimecode.cpp
  mrvTimelineSlider.cpp
  mrvToolGroup.cpp
  mrvToolWindow.cpp
  mrvVectorscope.cpp
  mrvVersion.cpp
  mrvVolumeSlider.cpp
  
)


if ( APPLE )
  list( PREPEND SOURCES mrvMainWindow.mm )
endif()

add_library(mrvWidgets ${SOURCES} ${HEADERS})

list( APPEND LIBRARIES mrvCore fltk Boost::regex )

if ( UNIX)
  if ( NOT APPLE )
    add_definitions( -DFLTK_USE_X11 )
    list(APPEND LIBRARIES Xss )
  endif()
endif()

target_link_libraries(mrvWidgets PUBLIC ${LIBRARIES} )
set_target_properties(mrvWidgets PROPERTIES FOLDER lib)
set_target_properties(mrvWidgets PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvWidgets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvWidgets)
