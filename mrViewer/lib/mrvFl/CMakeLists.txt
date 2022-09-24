set( FL_SOURCES
  keyboard_ui.fl
  mrvAboutUI.fl
  mrvHotkeyUI.fl
  mrvPreferencesUI.fl
  mrViewer.fl
  )

#set( USE_FLTK_RUN_FLUID_MACRO 1 )
if ( USE_FLTK_RUN_FLUID_MACRO )
  FLTK_RUN_FLUID( mrvFl_FLTK_UI_SRCS "${FL_SOURCES}" )
else()
  FLTK_WRAP_UI( mrvFl ${FL_SOURCES} )
endif()

set(HEADERS
  ../mrvGL/mrvThumbnailCreator.h

  FLU/flu_file_chooser_pixmaps.h
  FLU/flu_export.h
  FLU/flu_pixmaps.h
  FLU/Flu_Button.h
  FLU/Flu_Combo_Box.h
  FLU/Flu_Combo_List.h
  FLU/Flu_Combo_Tree.h
  FLU/Flu_Enumerations.h
  FLU/Flu_Label.h
  FLU/Flu_File_Chooser.h
  FLU/Flu_Return_Button.h
  FLU/Flu_Separator.h
  FLU/Flu_Wrap_Group.h

  mrvActionWindow.h
  mrvAsk.h
  mrvBrowser.h
  mrvButton.h
  mrvCallbacks.h
  mrvChoice.h
  mrvColorSchemes.h
  mrvContextObject.h
  mrvFileRequester.h

  mrvHotkey.h
  mrvIO.h
  mrvLanguages.h
  mrvMainWindow.h
  mrvPack.h
  mrvPopupMenu.h
  mrvPreferences.h
  mrvSlider.h
  mrvTextDisplay.h
  mrvTimecode.h
  mrvTimelineSlider.h
  mrvTimelinePlayer.h
  mrvTimeObject.h
  mrvTree.h
  mrvVersion.h
  mrvVolumeSlider.h

  resource.h
  )

set(SOURCES
  ${mrvFl_FLTK_UI_SRCS}

  ../mrvGL/mrvThumbnailCreator.cpp

  FLU/flu_file_chooser_pixmaps.cpp
  FLU/flu_pixmaps.cpp
  FLU/Flu_Button.cpp
  FLU/Flu_Combo_Box.cpp
  FLU/Flu_Combo_List.cpp
  FLU/Flu_Combo_Tree.cpp
  FLU/Flu_Label.cpp
  FLU/Flu_File_Chooser.cpp
  FLU/Flu_Return_Button.cpp
  FLU/Flu_Separator.cpp
  FLU/Flu_Wrap_Group.cpp

  mrvActionWindow.cpp
  mrvAsk.cpp
  mrvBrowser.cpp
  mrvButton.cpp
  mrvCallbacks.cpp
  mrvChoice.cpp
  mrvColorSchemes.cpp
  mrvContextObject.cpp
  mrvFileRequester.cpp
  mrvHotkey.cpp
  mrvIO.cpp
  mrvLanguages.cpp
  mrvMainWindow.cpp
  mrvMenus.cpp
  mrvPack.cpp
  mrvPopupMenu.cpp
  mrvPreferences.cpp
  mrvSlider.cpp
  mrvTextDisplay.cpp
  mrvTimecode.cpp
  mrvTimeObject.cpp
  mrvTimelinePlayer.cpp
  mrvTimelineSlider.cpp
  mrvTree.cpp
  mrvVersion.cpp
  mrvVolumeSlider.cpp
  )

set( LIBRARIES )
if ( UNIX)
  if ( NOT APPLE )
    add_definitions( -DFLTK_USE_X11 )
    list(APPEND LIBRARIES Xss )
  else()
    list( PREPEND SOURCES mrvMainWindow.mm )
  endif()
endif()



add_library(mrvFl ${SOURCES} ${HEADERS})

list( APPEND LIBRARIES mrvCore fltk )

target_link_libraries(mrvFl ${LIBRARIES} )
set_target_properties(mrvFl PROPERTIES FOLDER lib)
set_target_properties(mrvFl PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvFl
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvFl)
