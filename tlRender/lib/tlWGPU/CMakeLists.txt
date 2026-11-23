set(HEADERS
    Buffer.h
    Init.h
    Mesh.h
    OffscreenBuffer.h
    PipelineCreationState.h
    Shader.h
    ShaderInline.h
    ShaderBindingSet.h
    Texture.h
    TextureAtlas.h
    VAOPool.h
    WGPU.h
)
set(PRIVATE_HEADERS)

set(SOURCE
    Buffer.cpp
    Init.cpp
    Mesh.cpp
    OffscreenBuffer.cpp
    Shader.cpp
    ShaderBindingSet.cpp
    Texture.cpp
    TextureAtlas.cpp
    VAOPool.cpp)


set(LIBRARIES tlCore)
set(LIBRARIES_PRIVATE )

add_library(tlWGPU ${HEADERS} ${SOURCE})
add_compile_definitions(tlWGPU TLRENDER_WGPU_DAWN)
target_link_libraries(tlWGPU PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlWGPU PROPERTIES FOLDER lib)
set_target_properties(tlWGPU PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlWGPU
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlWGPU)
