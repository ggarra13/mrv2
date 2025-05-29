set(HEADERS
    ITest.h
    ITestInline.h)

set(SOURCE
    ITest.cpp)

add_library(tlTestLib ${SOURCE} ${HEADERS})
target_link_libraries(tlTestLib tlCore)
set_target_properties(tlTestLib PROPERTIES FOLDER tests)
