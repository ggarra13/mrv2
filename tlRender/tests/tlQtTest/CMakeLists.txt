set(HEADERS
    TimeObjectTest.h)

set(SOURCE
    TimeObjectTest.cpp)

add_library(tlQtTest ${SOURCE} ${HEADERS})
target_link_libraries(tlQtTest tlTestLib tlQt)
set_target_properties(tlQtTest PROPERTIES FOLDER tests)
