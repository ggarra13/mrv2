set(HEADERS
    Init.h
    Mesh.h
    OffscreenBuffer.h
    Shader.h
    Texture.h
    TextureAtlas.h
    Util.h
    Vk.h)
set(PRIVATE_HEADERS)

set(SOURCE
    Init.cpp
    Mesh.cpp
    OffscreenBuffer.cpp
    Shader.cpp
    Texture.cpp
    TextureAtlas.cpp
    Util.cpp)

find_package(Vulkan REQUIRED)
find_package(FLTK REQUIRED)

set(LIBRARIES tlCore)
set(LIBRARIES_PRIVATE Vulkan::Vulkan)

add_library(tlVk ${HEADERS} ${SOURCE})
target_link_libraries(tlVk ${LIBRARIES} ${LIBRARIES_PRIVATE})
set_target_properties(tlVk PROPERTIES FOLDER lib)
set_target_properties(tlVk PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlVk
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlVk)
