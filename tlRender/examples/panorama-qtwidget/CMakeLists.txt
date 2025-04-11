set(HEADERS
    PanoramaTimelineViewport.h)

set(SOURCE
    PanoramaTimelineViewport.cpp
    main.cpp)

add_executable(panorama-qtwidget ${SOURCE} ${HEADERS})
target_link_libraries(panorama-qtwidget tlQtWidget)
set_target_properties(panorama-qtwidget PROPERTIES FOLDER examples)
