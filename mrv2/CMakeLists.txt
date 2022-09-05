set(HEADERS
  App.h
  Util.h)
set(SOURCES
  App.cpp
  Util.cpp
  main.cpp)

set(LIBRARIES tlApp mrvFlGL mrvFl )

add_executable(mrv2 ${SOURCES} ${HEADERS})
target_link_libraries(mrv2 PUBLIC ${LIBRARIES} )

install(TARGETS mrv2
  RUNTIME DESTINATION bin)
