# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
  mrvColorSpaces.h
  mrvCPU.h
  mrvHome.h
  mrvHotkey.h
  mrvI8N.h
  mrvMedia.h
  mrvMesh.h
  mrvRoot.h
  mrvSequence.h
  mrvString.h
  mrvUtil.h
  )

set(SOURCES
  mrvColorSpaces.cpp
  mrvCPU.cpp
  mrvHome.cpp
  mrvHotkey.cpp
  mrvMedia.cpp
  mrvMesh.cpp
  mrvRoot.cpp
  mrvSequence.cpp
  mrvString.cpp
  mrvUtil.cpp
  )

if( WIN32 )
    list( APPEND HEADERS mrvOS.h )
    list( APPEND SOURCES mrvOS.cpp )
endif()

set(LIBRARIES tlTimeline tlDevice tlIO tlCore OCIO OTIO Imath
    Boost::filesystem )
set(LIBRARIES_PRIVATE RtAudio FSeq FreeType fltk )
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

files_to_absolute_paths()

target_link_libraries(mrvCore PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvCore PROPERTIES FOLDER lib)
set_target_properties(mrvCore PROPERTIES PUBLIC_HEADER "${HEADERS}")



install(TARGETS mrvCore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvCore
    COMPONENT libraries )
