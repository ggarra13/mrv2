set(HEADERS
    Render.h)
set(PRIVATE_HEADERS
    RenderPrivate.h
    RenderStructs.h)

set(SOURCE
    Render.cpp
    RenderPrims.cpp
    RenderVideo.cpp
    RenderVulkan.cpp
    RenderShaders.cpp)

set(LIBRARIES_PRIVATE )
if(TLRENDER_LIBPLACEBO)
    list(APPEND LIBRARIES_PRIVATE libplacebo)
endif()

find_package(Vulkan REQUIRED)
set(LIBRARIES_PRIVATE Vulkan::Vulkan)

add_library(tlTimelineVk ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
set_source_files_properties(Render.cpp PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

target_link_libraries(tlTimelineVk PUBLIC tlTimeline PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlTimelineVk PROPERTIES FOLDER lib)
set_target_properties(tlTimelineVk PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlTimelineVk
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimelineVk)
