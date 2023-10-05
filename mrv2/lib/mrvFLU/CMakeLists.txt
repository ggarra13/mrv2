# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
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
files_to_absolute_paths()

set( LIBRARIES ${FLTK_LIBRARIES} )

if(WIN32 AND FLTK_BUILD_SHARED_LIBS)
    target_compile_definitions(mrvFLU PUBLIC FL_DLL)
endif()

target_link_libraries(mrvFLU PUBLIC ${LIBRARIES} )
set_target_properties(mrvFLU PROPERTIES FOLDER lib)

install(TARGETS mrvFLU
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv2/mrvFLU
    COMPONENT libraries)
