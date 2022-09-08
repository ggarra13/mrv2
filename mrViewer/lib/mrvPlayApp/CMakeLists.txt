set(HEADERS
    App.h
    Util.h
  )
  set(SOURCES
    App.cpp
    Util.cpp
  )

add_library(mrvPlayApp ${SOURCES} ${HEADERS})
target_link_libraries(mrvPlayApp PRIVATE tlApp mrvFlGL mrvFl )
target_include_directories(mrvPlayApp PUBLIC ${INCLUDES} )
set_target_properties(mrvPlayApp PROPERTIES FOLDER lib)
set_target_properties(mrvPlayApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvPlayApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvPlayApp)
