include(ExternalProject)

set(Imath_GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/Imath.git")
set(Imath_GIT_TAG "v3.1.9")

set(Imath_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DBUILD_TESTING=OFF)

ExternalProject_Add(
    Imath
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/Imath
    GIT_REPOSITORY ${Imath_GIT_REPOSITORY}
    GIT_TAG ${Imath_GIT_TAG}
    
    LIST_SEPARATOR |
    CMAKE_ARGS ${Imath_ARGS})
