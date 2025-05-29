set(HEADERS
    AppTest.h
    CmdLineTest.h)

set(SOURCE
    AppTest.cpp
    CmdLineTest.cpp)

add_library(tlAppTest ${SOURCE} ${HEADERS})
target_link_libraries(tlAppTest tlTestLib tlBaseApp)
set_target_properties(tlAppTest PROPERTIES FOLDER tests)
