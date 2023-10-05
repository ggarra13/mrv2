# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    Flmm_ColorA_Button.h
    Flmm_ColorA_Chooser.h
)

set(SOURCES
    Flmm_ColorA_Button.cxx
    Flmm_ColorA_Chooser.cxx
)


add_library(mrvFlmm ${SOURCES} ${HEADERS})
files_to_absolute_paths()

set( LIBRARIES ${FLTK_LIBRARIES} )

target_compile_definitions(mrvFlmm PUBLIC FL_DLL)
target_link_libraries(mrvFlmm PUBLIC ${LIBRARIES} )
set_target_properties(mrvFlmm PROPERTIES FOLDER lib)
#set_target_properties(mrvFlmm PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvFlmm
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv2/mrvFlmm
    COMPONENT libraries)
