set(HEADERS
    App.h)

set(SOURCE
    App.cpp)

add_library(tlResourceApp ${HEADERS} ${SOURCE})
target_link_libraries(tlResourceApp tlIO tlBaseApp)
set_target_properties(tlResourceApp PROPERTIES FOLDER lib)
set_target_properties(tlResourceApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlResourceApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlResourceApp)
