set( FL_SOURCES
  make_ocio_chooser.fl
  keyboard_ui.fl
  mrvAboutUI.fl
  mrvHotkeyUI.fl
  mrvPreferencesUI.fl
  mrvImageInfo.fl
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
  mrvCollapsibleGroup.h
  mrvDockGroup.h
  mrvDragButton.h
  mrvDropWindow.h
  mrvMainWindow.h
  mrvOCIOBrowser.h
  mrvPack.h
  mrvPopupMenu.h
  mrvPreferencesTree.h
  mrvResizableBar.h
  mrvSlider.h
  mrvTable.h
  mrvTextDisplay.h
  mrvToolGroup.h
  mrvToolWindow.h
  mrvVersion.h
  mrvVolumeSlider.h
  )

set(SOURCES
  ${mrvWidgets_FLTK_UI_SRCS}

  mrvBrowser.cpp
  mrvButton.cpp
  mrvCollapsibleGroup.cpp
  mrvDockGroup.cpp
  mrvDragButton.cpp
  mrvDropWindow.cpp
  mrvMainWindow.cpp
  mrvOCIOBrowser.cpp
  mrvPack.cpp
  mrvPopupMenu.cpp
  mrvPreferencesTree.cpp
  mrvResizableBar.cpp
  mrvSlider.cpp
  mrvTable.cpp
  mrvTextDisplay.cpp
  mrvToolGroup.cpp
  mrvToolWindow.cpp
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
