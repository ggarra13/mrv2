include(ExternalProject)

# https://stackoverflow.com/questions/45414507/pass-a-list-of-prefix-paths-to-externalproject-add-in-cmake-args
string(REPLACE ";" "|" TLRENDER_EXTERNAL_DEPS_TMP "${TLRENDER_EXTERNAL_DEPS}")

set(tests_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DTLRENDER_EXTERNAL_DEPS=${TLRENDER_EXTERNAL_DEPS_TMP})
if(CMAKE_CXX_STANDARD)
    list(APPEND tests_ARGS -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD})
endif()

ExternalProject_Add(
    tests
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/tests
    DEPENDS ${TLRENDER_EXTERNAL_DEPS}
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/tests
    TEST_BEFORE_INSTALL TRUE
    INSTALL_COMMAND ""
    LIST_SEPARATOR |
    CMAKE_ARGS ${tests_ARGS})
