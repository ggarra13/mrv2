add_executable(tlffmpeg-decode-test main.cpp)
target_link_libraries(tlffmpeg-decode-test PRIVATE tlIO)

set(FAILURE_TEST_ARGS)
if(APPLE)
    add_executable(tlffmpeg-decode-failure-test main.cpp Failures.cpp)
    target_link_libraries(tlffmpeg-decode-failure-test PRIVATE tlIO)
    add_dependencies(tlffmpeg-decode-test tlffmpeg-decode-failure-test)
    list(APPEND FAILURE_TEST_ARGS
        --failure-decoder $<TARGET_FILE:tlffmpeg-decode-failure-test>)
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_program(FFMPEG_TEST_EXECUTABLE ffmpeg REQUIRED)
add_test(
    NAME FFmpegHardwareDecode
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run.py
        --ffmpeg ${FFMPEG_TEST_EXECUTABLE}
        --decoder $<TARGET_FILE:tlffmpeg-decode-test> ${FAILURE_TEST_ARGS})
set_tests_properties(FFmpegHardwareDecode PROPERTIES TIMEOUT 180)
