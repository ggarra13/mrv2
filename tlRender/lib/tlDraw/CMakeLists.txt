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



set(LIBRARIES tlCore )

add_library(tlDraw ${SOURCES} ${HEADERS})

target_link_libraries(tlDraw PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(tlDraw PROPERTIES FOLDER lib)


install(TARGETS tlDraw
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)
