# SPDX-License-Identifier: BSD-3-Clause
# mrv2 (mrViewer2)
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvAnnotationsTool.h
    mrvColorAreaTool.h
    mrvColorTool.h
    mrvCompareTool.h
    mrvDevicesTool.h
    mrvHistogramTool.h
    mrvImageInfoTool.h
    mrvEnvironmentMapTool.h
    mrvLogsTool.h
    mrvPlaylistTool.h
    mrvFilesTool.h
    mrvSettingsTool.h
    mrvToolsCallbacks.h
    mrvToolWidget.h
)

set(SOURCES
    mrvAnnotationsTool.cpp
    mrvColorAreaTool.cpp
    mrvColorTool.cpp
    mrvCompareTool.cpp
    mrvDevicesTool.cpp
    mrvHistogramTool.cpp
    mrvImageInfoTool.cpp
    mrvEnvironmentMapTool.cpp
    mrvLogsTool.cpp
    mrvPlaylistTool.cpp
    mrvFilesTool.cpp
    mrvSettingsTool.cpp
    mrvToolsCallbacks.cpp
    mrvToolWidget.cpp
    mrvVectorscopeTool.cpp
)


add_library(mrvTools ${SOURCES} ${HEADERS})

set( LIBRARIES mrvGL mrvFl )

target_link_libraries(mrvTools PUBLIC ${LIBRARIES} )
set_target_properties(mrvTools PROPERTIES FOLDER lib)
set_target_properties(mrvTools PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvTools
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvTools
    COMPONENT libraries)
