set(HEADERS
    Render.h
    RenderOptions.h
    RenderPrivate.h
    RenderStructs.h
)
set(PRIVATE_HEADERS
    # HDRPeakDetection.h
)

set(SOURCE
    # HDRPeakDetection.cpp
    Render.cpp
    RenderAnaglyph.cpp
    RenderPrims.cpp
    RenderVideo.cpp
    RenderWebGPU.cpp
    RenderShaders.cpp
    RenderStereo.cpp
)

set(LIBRARIES_PRIVATE )

add_library(tlTimelineWGPU ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlTimelineWGPU PUBLIC tlWGPU tlTimeline PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlTimelineWGPU PROPERTIES FOLDER lib)
set_target_properties(tlTimelineWGPU PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlTimelineWGPU
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimelineWGPU)
