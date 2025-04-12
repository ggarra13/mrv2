include(ExternalProject)


set(jasper_GIT_REPOSITORY "https://github.com/jasper-software/jasper.git")
set(jasper_GIT_TAG version-4.2.1)

set(jasper_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DBUILD_SHARED_LIBS=OFF
    -DJAS_ENABLE_SHARED=OFF
    -DJAS_ENABLE_PROGRAMS=OFF
    -DJAS_ENABLE_DOC=OFF
    -DJAS_ENABLE_OPENGL=OFF
)


set(jasper_DEPS )
if(TLRENDER_JPEG)
    list(APPEND jasper_DEPS libjpeg-turbo)
endif()
    
ExternalProject_Add(
     jasper
     PREFIX ${CMAKE_CURRENT_BINARY_DIR}/jasper
     GIT_REPOSITORY ${jasper_GIT_REPOSITORY}
     GIT_TAG ${jasper_GIT_TAG}
     
     DEPENDS ${jasper_DEPS}
     LIST_SEPARATOR |
     CMAKE_ARGS ${jasper_ARGS}
)
