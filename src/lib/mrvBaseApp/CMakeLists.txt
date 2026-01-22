# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvBaseApp.h
    mrvCmdLine.h
    mrvCmdLineInline.h
)
set(SOURCES
    mrvBaseApp.cpp
    mrvCmdLine.cpp
)

add_library(mrvBaseApp ${SOURCES} ${HEADERS})
files_to_absolute_paths()

find_package(Freetype REQUIRED)
find_package(Imath REQUIRED)
find_package(RtAudio REQUIRED)

target_link_libraries(mrvBaseApp tlCore RtAudio Freetype::Freetype Imath::Imath)
target_include_directories(mrvBaseApp PUBLIC ${INCLUDES} )
set_target_properties(mrvBaseApp PROPERTIES FOLDER lib)


install(TARGETS mrvBaseApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrvBaseApp
    COMPONENT libraries )
