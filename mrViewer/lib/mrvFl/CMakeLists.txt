

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

  
  mrvAsk.h
  mrvCallbacks.h
  mrvChoice.h
  mrvClipButton.h
  mrvColorSchemes.h
  mrvColorTool.h
  mrvCompareTool.h
  mrvContextObject.h
  mrvFileRequester.h
  mrvHorSlider.h
  mrvHotkey.h
  mrvImageInformation.h
  mrvIO.h
  mrvLanguages.h
  mrvPreferences.h
  mrvFilesTool.h
  mrvSettingsTool.h
  mrvTimecode.h
  mrvTimelineSlider.h
  mrvTimelinePlayer.h
  mrvTimeObject.h
  mrvToolsCallbacks.h
  mrvToolWidget.h
  mrvUtil.h

  )

set(SOURCES

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
  
  mrvAsk.cpp
  mrvCallbacks.cpp
  mrvChoice.cpp
  mrvClipButton.cpp
  mrvColorSchemes.cpp
  mrvColorTool.cpp
  mrvCompareTool.cpp
  mrvContextObject.cpp
  mrvFileRequester.cpp
  mrvHorSlider.cpp
  mrvHotkey.cpp
  mrvImageInformation.cpp
  mrvIO.cpp
  mrvLanguages.cpp
  mrvMenus.cpp
  mrvPreferences.cpp
  mrvFilesTool.cpp
  mrvSettingsTool.cpp
  mrvTimecode.cpp
  mrvTimeObject.cpp
  mrvTimelinePlayer.cpp
  mrvTimelineSlider.cpp
  mrvToolsCallbacks.cpp
  mrvToolWidget.cpp
  mrvUtil.cpp
  )

   
add_library(mrvFl ${SOURCES} ${HEADERS})

set( LIBRARIES mrvCore mrvWidgets fltk fltk_images Boost::regex )
if (WIN32)
   list( APPEND LIBRARIES comsuppw )
endif()

target_link_libraries(mrvFl PUBLIC ${LIBRARIES} )
set_target_properties(mrvFl PROPERTIES FOLDER lib)
set_target_properties(mrvFl PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvFl
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvFl)
