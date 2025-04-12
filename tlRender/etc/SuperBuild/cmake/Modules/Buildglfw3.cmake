include(ExternalProject)

set(glfw3_GIT_REPOSITORY "https://github.com/glfw/glfw.git")
#set(glfw3_GIT_TAG 3eaf1255b29fdf5c2895856c7be7d7185ef2b241)  # used to work
set(glfw3_GIT_TAG "3.4")  # used to work

set(glfw3_Linux_ARGS )
if(UNIX AND NOT APPLE)
    list(APPEND glfw3_Linux_ARGS
	-DGLFW_BUILD_X11=${TLRENDER_X11}
	-DGLFW_BUILD_WAYLAND=${TLRENDER_WAYLAND}
    )
endif()

set(glfw3_PATCH)
if(TLRENDER_GLFW_DISABLE_MACOS_APP_DELEGATE)
    list(APPEND glfw3_PATCH
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/glfw3-patch/src/cocoa_init.m
        ${CMAKE_CURRENT_BINARY_DIR}/glfw3/src/glfw3/src/cocoa_init.m)
endif()

set(glfw3_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    ${glfw3_Linux_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DGLFW_BUILD_EXAMPLES=FALSE
    -DGLFW_BUILD_TESTS=FALSE
    -DGLFW_BUILD_DOCS=FALSE)

ExternalProject_Add(
    glfw3
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/glfw3
    GIT_REPOSITORY ${glfw3_GIT_REPOSITORY}
    GIT_TAG ${glfw3_GIT_TAG}
    PATCH_COMMAND ${glfw3_PATCH}
    LIST_SEPARATOR |
    CMAKE_ARGS ${glfw3_ARGS})

