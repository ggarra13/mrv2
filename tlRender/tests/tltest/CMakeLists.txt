set(HEADERS)

set(SOURCE
    main.cpp)

set(LIBRARIES
    tlAppTest
    tlCoreTest
    tlGLTest
    tlIOTest
    tlTimelineTest)
if(TLRENDER_QT6 OR TLRENDER_QT5 AND NOT "${TLRENDER_API}" STREQUAL "GLES_2")
    list(APPEND LIBRARIES tlQtTest)
endif()

add_executable(tltest ${SOURCE} ${HEADERS})
target_link_libraries(tltest ${LIBRARIES})
set_target_properties(tltest PROPERTIES FOLDER tests)

add_test(tltest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tltest${CMAKE_EXECUTABLE_SUFFIX})
