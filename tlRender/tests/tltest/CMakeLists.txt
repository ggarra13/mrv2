set(HEADERS)

set(SOURCE
    main.cpp)

set(LIBRARIES
    tlCoreTest
    tlIOTest
    tlTimelineTest
    tlDevice
)

if(NOT MRV2_BACKEND STREQUAL "VK")
    list(APPEND LIBRARIES tlGLTest)
endif()

find_package(NDI)

add_executable(tltest ${SOURCE} ${HEADERS})
target_link_libraries(tltest ${LIBRARIES})
set_target_properties(tltest PROPERTIES FOLDER tests)

add_test(tltest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tltest${CMAKE_EXECUTABLE_SUFFIX})
