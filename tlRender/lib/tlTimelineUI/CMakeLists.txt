set(HEADERS
    AudioClipItem.h
    GapItem.h
    IBasicItem.h
    IItem.h
    Init.h
    TimelineItem.h
    TimelineViewport.h
    TimelineWidget.h
    TransitionItem.h
    VideoClipItem.h)
set(HEADERS_PRIVATE
    TimelineItemPrivate.h)

set(SOURCE
    AudioClipItem.cpp
    GapItem.cpp
    IBasicItem.cpp
    IItem.cpp
    Init.cpp
    TimelineItem.cpp
    TimelineViewport.cpp
    TimelineWidget.cpp
    TransitionItem.cpp
    VideoClipItem.cpp)

add_library(tlTimelineUI ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlTimelineUI PUBLIC tlUI PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlTimelineUI PROPERTIES FOLDER lib)
set_target_properties(tlTimelineUI PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlTimelineUI
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimelineUI)
