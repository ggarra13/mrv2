find_package(SampleRate REQUIRED)

set(source main.cpp)

add_executable(SampleRateTest ${header} ${source})
target_link_libraries(SampleRateTest SampleRate::samplerate)

add_test(SampleRateTest SampleRateTest)
