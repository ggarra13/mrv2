include(ExternalProject)

set(SampleRate_GIT_REPOSITORY "https://github.com/libsndfile/libsamplerate.git")
set(SampleRate_GIT_TAG "0.2.2")

set(SampleRate_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DBUILD_TESTING=OFF
    -DLIBSAMPLERATE_EXAMPLES=OFF)

ExternalProject_Add(
    SampleRate
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/SampleRate
    GIT_REPOSITORY ${SampleRate_GIT_REPOSITORY}
    GIT_TAG ${SampleRate_GIT_TAG}
    
    LIST_SEPARATOR |
    CMAKE_ARGS ${SampleRate_ARGS})
