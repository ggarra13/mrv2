set(HEADERS
    include/glad/gl.h
    include/KHR/khrplatform.h)

set(SOURCE
    src/gl.c)

add_library(glad ${HEADERS} ${SOURCE})

set(LIBRARIES OpenGL::GL)
if (UNIX AND NOT APPLE)
    set(LIBRARIES OpenGL::OpenGL)
endif()
target_link_libraries(glad ${LIBRARIES} ${CMAKE_DL_LIBS})
set_target_properties(glad PROPERTIES FOLDER deps)

install(
    TARGETS glad
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
install(
    DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/include/glad"
    DESTINATION "include/tlRender")
install(
    DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/include/KHR"
    DESTINATION "include/tlRender")
