set(HEADERS
    App.h)

set(SOURCE
    App.cpp)

set(LIBRARIES tlTimelineGL tlBaseApp)

add_library(tlBakeApp ${HEADERS} ${SOURCE})
target_link_libraries(tlBakeApp ${LIBRARIES})
set_target_properties(tlBakeApp PROPERTIES FOLDER lib)
set_target_properties(tlBakeApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlBakeApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlBakeApp)
