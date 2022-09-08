set(HEADERS
    App.h
    Util.h
  )
  set(SOURCES
    App.cpp
    Util.cpp
  )

add_library(mrvFlApp ${SOURCES} ${HEADERS})
target_link_libraries(mrvFlApp PRIVATE tlApp mrvFlGL mrvFl )
target_include_directories(mrvFlApp PUBLIC ${INCLUDES} )
set_target_properties(mrvFlApp PROPERTIES FOLDER lib)
set_target_properties(mrvFlApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvFlApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvFlApp)
