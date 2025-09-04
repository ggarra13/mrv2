# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvAnnotation.h
    mrvVoiceData.h
    # mrvVoicePlayer.h
    mrvVoiceOver.h
)

set(SOURCES
    mrvAnnotation.cpp
    # mrvVoicePlayer.cpp
    mrvVoiceOver.cpp
)

set(LIBRARIES tlRender::tlCore ${FLTK_LIBRARIES} )
add_library(mrvVoice ${SOURCES} ${HEADERS})

files_to_absolute_paths()

target_link_libraries(mrvVoice PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvVoice PROPERTIES FOLDER lib)


install(TARGETS mrvVoice
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries )
