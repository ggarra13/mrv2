# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    flu_file_chooser_pixmaps.h
    flu_export.h
    flu_pixmaps.h
    Flu_Button.h
    Flu_Combo_Box.h
    Flu_Combo_List.h
    Flu_Combo_Tree.h
    Flu_Enumerations.h
    Flu_Label.h
    Flu_File_Chooser.h
    Flu_Return_Button.h
    Flu_Separator.h
    Flu_Wrap_Group.h
)

set(SOURCES
    flu_file_chooser_pixmaps.cpp
    flu_pixmaps.cpp
    Flu_Button.cpp
    Flu_Combo_Box.cpp
    Flu_Combo_List.cpp
    Flu_Combo_Tree.cpp
    Flu_Label.cpp
    Flu_File_Chooser.cpp
    Flu_Return_Button.cpp
    Flu_Separator.cpp
    Flu_Wrap_Group.cpp
)


add_library(mrvFLU ${SOURCES} ${HEADERS})

set( LIBRARIES fltk fltk_images )

target_link_libraries(mrvFLU PUBLIC ${LIBRARIES} )
set_target_properties(mrvFLU PROPERTIES FOLDER lib)
set_target_properties(mrvFLU PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvFLU
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvFLU
    COMPONENT libraries)
