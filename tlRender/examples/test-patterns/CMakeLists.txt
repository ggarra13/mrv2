set(HEADERS
    App.h
    TestPatterns.h)

set(SOURCE
    App.cpp
    TestPatterns.cpp
    main.cpp)

add_executable(test-patterns ${SOURCE} ${HEADERS})
target_link_libraries(test-patterns tlTimelineGL tlBaseApp)
set_target_properties(test-patterns PROPERTIES FOLDER examples)
