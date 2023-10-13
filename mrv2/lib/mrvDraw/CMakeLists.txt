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
    Point.cpp
    Polyline2D.cpp
    Shape.cpp
    )



set(LIBRARIES mrvCore )

add_library(mrvDraw ${SOURCES} ${HEADERS})
files_to_absolute_paths()

target_link_libraries(mrvDraw PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvDraw PROPERTIES FOLDER lib)



install(TARGETS mrvDraw
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)
