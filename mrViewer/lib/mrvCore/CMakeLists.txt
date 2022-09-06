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

set(LIBRARIES tlTimeline tlDevice tlIO tlCore OCIO OTIO Imath
  Boost::filesystem )
set(LIBRARIES_PRIVATE RtAudio FSeq FreeType )
if ( FFmpeg_FOUND )
  list( APPEND LIBRARIES_PRIVATE FFmpeg )
endif()
if ( JPEG_FOUND )
  list( APPEND LIBRARIES_PRIVATE JPEG )
endif()
if ( PNG_FOUND )
  list( APPEND LIBRARIES_PRIVATE PNG )
endif()
if ( TIFF_FOUND )
  list( APPEND LIBRARIES_PRIVATE TIFF )
endif()
if ( OpenEXR_FOUND )
  list( APPEND LIBRARIES_PRIVATE OpenEXR )
endif()

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
