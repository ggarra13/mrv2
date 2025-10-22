include(ExternalProject)

set(expat_GIT_REPOSITORY "https://github.com/libexpat/libexpat.git")
set(expat_GIT_TAG "R_2_7_2")  # was 2_7_1

set(expat_DEPENDENCIES )
message(STATUS "expat DEPENDENCIES=${expat_DEPENDENCIES}")

set(expat_ARGS
    -DEXPAT_BUILD_TOOLS=OFF
    -DEXPAT_BUILD_EXAMPLES=OFF
    -DEXPAT_BUILD_TESTS=OFF
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    expat
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/expat
    GIT_REPOSITORY ${expat_GIT_REPOSITORY}
    GIT_TAG ${expat_GIT_TAG}

    DEPENDS ${expat_DEPENDENCIES}
    
    SOURCE_SUBDIR expat
    LIST_SEPARATOR |
    CMAKE_ARGS ${expat_ARGS})
