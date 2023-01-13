# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS

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


  mrvAnnotationsTool.h
  mrvAsk.h
  mrvCallbacks.h
  mrvColorAreaTool.h
  mrvColorSchemes.h
  mrvColorTool.h
  mrvCompareTool.h
  mrvContextObject.h
  mrvDevicesTool.h
  mrvFileRequester.h
  mrvHistogramTool.h
  mrvHotkey.h
  mrvImageInfoTool.h
  mrvIO.h
  mrvLanguages.h
  mrvLogsTool.h
  mrvMenus.h
  mrvPreferences.h
  mrvFilesTool.h
  mrvSaving.h
  mrvSettingsTool.h
  mrvTimelinePlayer.h
  mrvTimeObject.h
  mrvToolsCallbacks.h
  mrvToolWidget.h
  mrvUtil.h
  mrvVectorscopeTool.h
  mrvVersioning.h

)

set(SOURCES


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

  mrvAnnotationsTool.cpp
  mrvAsk.cpp
  mrvCallbacks.cpp
  mrvColorAreaTool.cpp
  mrvColorSchemes.cpp
  mrvColorTool.cpp
  mrvCompareTool.cpp
  mrvContextObject.cpp
  mrvDevicesTool.cpp
  mrvFileRequester.cpp
  mrvHistogramTool.cpp
  mrvHotkey.cpp
  mrvImageInfoTool.cpp
  mrvIO.cpp
  mrvLanguages.cpp
  mrvLogsTool.cpp
  mrvMenus.cpp
  mrvPreferences.cpp
  mrvFilesTool.cpp
  mrvSaving.cpp
  mrvSettingsTool.cpp
  mrvTimeObject.cpp
  mrvTimelinePlayer.cpp
  mrvToolsCallbacks.cpp
  mrvToolWidget.cpp
  mrvUtil.cpp
  mrvVectorscopeTool.cpp
  mrvVersioning.cpp
  )


add_library(mrvFl ${SOURCES} ${HEADERS})

set( LIBRARIES mrvCore mrvWidgets mrvGL fltk fltk_images Boost::regex )
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
    PUBLIC_HEADER DESTINATION include/mrv/mrvFl
    COMPONENT libraries)
