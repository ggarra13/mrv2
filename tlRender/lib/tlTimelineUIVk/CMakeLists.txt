set(HEADERS
    AudioClipItem.h
    GapItem.h
    IBasicItem.h
    IItem.h
    Init.h
    ThumbnailSystem.h
    TimelineItem.h
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
    ThumbnailSystem.cpp
    TimelineItem.cpp
    TimelineItemFill.cpp
    TimelineItemMove.cpp
    TimelineItemRipple.cpp
    TimelineItemRoll.cpp
    TimelineItemSlice.cpp
    TimelineItemSlide.cpp
    TimelineItemSlip.cpp
    TimelineItemTrim.cpp
    TimelineWidget.cpp
    TransitionItem.cpp
    VideoClipItem.cpp)

add_library(tlTimelineUIVk ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlTimelineUIVk PUBLIC tlUI PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlTimelineUIVk PROPERTIES FOLDER lib)
set_target_properties(tlTimelineUIVk PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlTimelineUIVk
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimelineUIVk)
