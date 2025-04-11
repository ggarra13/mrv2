set(HEADERS
    App.h
    Window.h)

set(SOURCE
    App.cpp
    Window.cpp)

set(LIBRARIES tlUI tlBaseApp)

add_library(tlUIApp ${HEADERS} ${SOURCE})
target_link_libraries(tlUIApp ${LIBRARIES})
set_target_properties(tlUIApp PROPERTIES FOLDER lib)
set_target_properties(tlUIApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlUIApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlUIApp)
