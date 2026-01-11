set(HEADERS
    AudioClipItem.h
    EffectItem.h
    GapItem.h
    IBasicItem.h
    IItem.h
    Init.h
    Namespace.h
    ThumbnailSystem.h
    TimelineItem.h
    TimelineWidget.h
    TransitionItem.h
    VideoClipItem.h)
set(HEADERS_PRIVATE
    TimelineItemPrivate.h)

set(SOURCE
    AudioClipItem.cpp
    EffectItem.cpp
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

if(MRV2_BACKEND STREQUAL "VK")
    set(LIBRARY_NAME tlTimelineUIVk)
endif()

if(MRV2_BACKEND STREQUAL "GL")
    set(LIBRARY_NAME tlTimelineUI)
endif()

add_library(${LIBRARY_NAME} ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(${LIBRARY_NAME} PUBLIC tlUI PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(${LIBRARY_NAME} PROPERTIES FOLDER lib)
set_target_properties(${LIBRARY_NAME} PROPERTIES PUBLIC_HEADER "${HEADERS}")
target_include_directories(${LIBRARY_NAME} 
    PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/tlRender/${LIBRARY_NAME}>
)
    
install(TARGETS ${LIBRARY_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/${LIBRARY_NAME})
