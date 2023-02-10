# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    Annotation.h
    LineSegment.h
    Point.h
    Polyline2D.h
    Shape.h
    )

set(SOURCES
    Annotation.cpp
    )



set(LIBRARIES mrvCore )

add_library(mrvDraw ${SOURCES} ${HEADERS})
files_to_absolute_paths()
set( PO_SOURCES ${PO_FILES} ${PO_SOURCES} PARENT_SCOPE)

target_link_libraries(mrvDraw PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvDraw PROPERTIES FOLDER lib)
set_target_properties(mrvDraw PROPERTIES PUBLIC_HEADER "${HEADERS}")



install(TARGETS mrvDraw
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvDraw
    COMPONENT libraries)
