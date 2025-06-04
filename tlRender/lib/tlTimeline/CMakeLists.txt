set(HEADERS
    Audio.h
    AudioInline.h
    BackgroundOptions.h
    BackgroundOptionsInline.h
    CompareOptions.h
    CompareOptionsInline.h
    DisplayOptions.h
    DisplayOptionsInline.h
    Edit.h
    HDROptions.h
    HDROptionsInline.h
    IRender.h
    ImageOptions.h
    ImageOptionsInline.h
    Init.h
    LUTOptions.h
    LUTOptionsInline.h
    MemoryReference.h
    OCIOOptions.h
    OCIOOptionsInline.h
    Player.h
    PlayerInline.h
    PlayerOptions.h
    PlayerOptionsInline.h
    RenderOptions.h
    RenderOptionsInline.h
    RenderUtil.h
    TimeUnits.h
    Timeline.h
    Transition.h
    Util.h
    UtilInline.h
    Video.h
    VideoInline.h)
set(PRIVATE_HEADERS
    PlayerPrivate.h
    TimelinePrivate.h)

set(SOURCE
    BackgroundOptions.cpp
    CompareOptions.cpp
    DisplayOptions.cpp
    Edit.cpp
    HDROptions.cpp
    IRender.cpp
    ImageOptions.cpp
    Init.cpp
    LUTOptions.cpp
    MemoryReference.cpp
    OCIOOptions.cpp
    Player.cpp
    PlayerAudio.cpp
    PlayerOptions.cpp
    PlayerPrivate.cpp
    RenderUtil.cpp
    TimeUnits.cpp
    Timeline.cpp
    TimelineCreate.cpp
    TimelinePrivate.cpp
    Transition.cpp
    Util.cpp)

add_library(tlTimeline ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlTimeline tlIO)
set_target_properties(tlTimeline PROPERTIES FOLDER lib)
set_target_properties(tlTimeline PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlTimeline
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimeline)
