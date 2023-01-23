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


    mrvAsk.h
    mrvCallbacks.h
    mrvColorSchemes.h
    mrvContextObject.h
    mrvFileRequester.h
    mrvHotkey.h
    mrvIO.h
    mrvLanguages.h
    mrvMenus.h
    mrvPreferences.h
    mrvSaving.h
    mrvTimelinePlayer.h
    mrvUtil.h
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

    mrvAsk.cpp
    mrvCallbacks.cpp
    mrvColorSchemes.cpp
    mrvContextObject.cpp
    mrvFileRequester.cpp
    mrvHotkey.cpp
    mrvIO.cpp
    mrvLanguages.cpp
    mrvMenus.cpp
    mrvPreferences.cpp
    mrvSaving.cpp
    mrvTimeObject.cpp
    mrvTimelineCreate.cpp
    mrvTimelinePlayer.cpp
    mrvUtil.cpp
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
