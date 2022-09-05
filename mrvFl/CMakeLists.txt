set( FL_SOURCES
  mrvPreferencesUI.fl
  keyboard_ui.fl
  mrvHotkeyUI.fl
  mrViewer.fl
  )

#set( USE_FLTK_RUN_FLUID_MACRO 1 )
if ( USE_FLTK_RUN_FLUID_MACRO )
  FLTK_RUN_FLUID( mrvFl_FLTK_UI_SRCS "${FL_SOURCES}" )
else()
  FLTK_WRAP_UI( mrvFl ${FL_SOURCES} )
endif()

set(HEADERS
  ../mrvCore/mrvPreferences.h
  ../mrvCore/mrvColorSchemes.h

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
  mrvButton.h
  mrvChoice.h
  mrvFileRequester.h

  mrvHotkey.h
  mrvIO.h
  mrvLanguages.h
  mrvMainWindow.h
  mrvPack.h
  mrvPopupMenu.h
  mrvSlider.h
  mrvTimecode.h
  mrvTimeline.h
  mrvTimelinePlayer.h
  mrvTree.h
  mrvVolumeSlider.h

  resource.h
  )

set(SOURCES
  ${mrvFl_FLTK_UI_SRCS}
  ../mrvCore/mrvPreferences.cpp
  ../mrvCore/mrvColorSchemes.cpp

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
  mrvButton.cpp
  mrvChoice.cpp
  mrvFileRequester.cpp
  mrvHotkey.cpp
  mrvIO.cpp
  mrvLanguages.cpp
  mrvMainWindow.cpp
  mrvPack.cpp
  mrvPopupMenu.cpp
  mrvSlider.cpp
  mrvTimecode.cpp
  mrvTimeline.cpp
  mrvTimelinePlayer.cpp
  mrvTree.cpp
  mrvVolumeSlider.cpp
  )

add_library(mrvFl ${SOURCES} ${HEADERS})

if ( UNIX AND NOT APPLE )
  add_definitions( -DFLTK_USE_X11 )
  set( OSLIBRARIES Xss )
endif()

target_link_libraries(mrvFl PRIVATE mrvCore fltk ${OSLIBRARIES} )
set_target_properties(mrvFl PROPERTIES FOLDER lib)
set_target_properties(mrvFl PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvFl
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvFl)
