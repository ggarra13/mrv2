set(HEADERS
    Buffer.h
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
    Buffer.cpp
    Init.cpp
    Mesh.cpp
    OffscreenBuffer.cpp
    Shader.cpp
    Texture.cpp
    TextureAtlas.cpp
    Util.cpp)

find_package(Vulkan REQUIRED)  
find_package(FLTK CONFIG REQUIRED)

set(LIBRARIES tlCore Vulkan::Vulkan)
set(LIBRARIES_PRIVATE )

add_library(tlVk ${HEADERS} ${SOURCE})
target_link_libraries(tlVk PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlVk PROPERTIES FOLDER lib)
set_target_properties(tlVk PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlVk
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlVk)
