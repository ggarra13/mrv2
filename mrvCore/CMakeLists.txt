#
# FLTK GUIs created with fluid
#
set(HEADERS
  mrvColor.h
  mrvColorSpaces.h
  mrvHome.h
  mrvHotkey.h
  mrvI8N.h
  mrvImagePixel.h
  mrvInteger.h
  mrvMedia.h
  mrvRectangle.h
  mrvSequence.h
  mrvString.h
  mrvTimeObject.h
  mrvUtil.h
  )

set(SOURCES
  mrvColorSpaces.cpp
  mrvHome.cpp
  mrvHotkey.cpp
  mrvMedia.cpp
  mrvSequence.cpp
  mrvString.cpp
  mrvTimeObject.cpp
  )

set( LIBRARIES tlCore Boost::filesystem )
set( LIBRARIES_PRIVATE  )

add_library(mrvCore ${SOURCES} ${HEADERS})
add_definitions( -DMR_SSE )

target_link_libraries(mrvCore PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvCore PROPERTIES FOLDER lib)
set_target_properties(mrvCore PROPERTIES PUBLIC_HEADER "${HEADERS}")



install(TARGETS mrvCore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvCore)
