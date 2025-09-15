include(ExternalProject)

set(OpenJPH_GIT_REPOSITORY "https://github.com/aous72/OpenJPH.git")
set(OpenJPH_GIT_TAG "0.23.0")

set(OpenJPH_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
)

set(OpenJPH_PATH )
if(WIN32)
    list(APPEND OpenJPH_PATCH COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/patches/OpenJPH-patch/src/core/common/ojph_arch.h
        ${CMAKE_CURRENT_BINARY_DIR}/OpenJPH/src/OpenJPH/src/core/common)
endif()

ExternalProject_Add(
    OpenJPH
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenJPH
    GIT_REPOSITORY ${OpenJPH_GIT_REPOSITORY}
    GIT_TAG ${OpenJPH_GIT_TAG}
    
    PATCH_COMMAND ${OpenJPH_PATCH}
    
    LIST_SEPARATOR |
    CMAKE_ARGS ${OpenJPH_ARGS})

set(OpenJPH_DEP OpenJPH)
