set(HEADERS
    Render.h)
set(PRIVATE_HEADERS
    RenderPrivate.h)

set(SOURCE
    Render.cpp
    RenderPrims.cpp
    RenderVideo.cpp)
if("${TLRENDER_API}" STREQUAL "GL_4_1" OR "${TLRENDER_API}" STREQUAL "GL_4_1_Debug")
    list(APPEND SOURCE RenderShaders_GL_4_1.cpp)
elseif("${TLRENDER_API}" STREQUAL "GLES_2")
    list(APPEND SOURCE RenderShaders_GLES_2.cpp)
endif()

set(LIBRARIES_PRIVATE )
if(TLRENDER_LIBPLACEBO)
    list(APPEND LIBRARIES_PRIVATE libplacebo)
endif()

add_library(tlTimelineGL ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
set_source_files_properties(Render.cpp PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

target_link_libraries(tlTimelineGL PUBLIC tlTimeline PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlTimelineGL PROPERTIES FOLDER lib)
set_target_properties(tlTimelineGL PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlTimelineGL
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimelineGL)
