# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvBaseApp.h
)
set(SOURCES
    mrvBaseApp.cpp
)

add_library(mrvBaseApp ${SOURCES} ${HEADERS})
files_to_absolute_paths()

find_package(Freetype REQUIRED)
find_package(Imath REQUIRED)
find_package(RtAudio REQUIRED)

target_link_libraries(mrvBaseApp tlBaseApp tlCore RtAudio Freetype::Freetype Imath::Imath)
target_include_directories(mrvBaseApp PUBLIC ${INCLUDES} )
set_target_properties(mrvBaseApp PROPERTIES FOLDER lib)


install(TARGETS mrvBaseApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries )
