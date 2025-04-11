set(HEADERS
    GLFWTest.h
    MeshTest.h
    OffscreenBufferTest.h
    ShaderTest.h
    TextureTest.h)

set(SOURCE
    GLFWTest.cpp
    MeshTest.cpp
    OffscreenBufferTest.cpp
    ShaderTest.cpp
    TextureTest.cpp)

add_library(tlGLTest ${SOURCE} ${HEADERS})
target_link_libraries(tlGLTest tlTestLib tlGL)
set_target_properties(tlGLTest PROPERTIES FOLDER tests)
