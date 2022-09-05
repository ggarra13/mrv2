set(HEADERS
  )
set(SOURCES
  )

add_library(tlFlApp ${SOURCES} ${HEADERS})
target_link_libraries(tlFlApp PRIVATE tlFlGL tlFl )
target_include_directories(tlFlApp PUBLIC ${INCLUDES} )
set_target_properties(tlFlApp PROPERTIES FOLDER lib)
set_target_properties(tlFlApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlFlApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/tlFlApp)
