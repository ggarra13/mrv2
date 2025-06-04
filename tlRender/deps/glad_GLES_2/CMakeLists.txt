set(HEADERS
    include/glad/glad.h
    include/KHR/khrplatform.h)

set(SOURCE
    src/glad.c)

add_library(glad ${HEADERS} ${SOURCE})
target_link_libraries(glad OpenGL::GL ${CMAKE_DL_LIBS})
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
