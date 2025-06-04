set(HEADERS
    CmdLine.h
    CmdLineInline.h
    BaseApp.h)

set(SOURCE
    CmdLine.cpp
    BaseApp.cpp)

add_library(tlBaseApp ${HEADERS} ${SOURCE})
target_link_libraries(tlBaseApp tlCore)
set_target_properties(tlBaseApp PROPERTIES FOLDER lib)
set_target_properties(tlBaseApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlBaseApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlBaseApp)
