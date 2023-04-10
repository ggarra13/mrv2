# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    Cmds.h
    CmdsAux.h
)
set(SOURCES
    Cmds.cpp
    Enums.cpp
    FileItem.cpp
    FilesModel.cpp
    FilePath.cpp
    Image.cpp
    Media.cpp
    OTIO.cpp
    Playlist.cpp
    Settings.cpp
    Timeline.cpp
    Vectors.cpp
)

add_library(mrvPy ${SOURCES} ${HEADERS})
files_to_absolute_paths()

target_link_libraries(mrvPy mrvApp )
target_include_directories(mrvPy PUBLIC ${INCLUDES} )
set_target_properties(mrvPy PROPERTIES FOLDER lib)
set_target_properties(mrvPy PROPERTIES PUBLIC_HEADER "${HEADERS}")



install(TARGETS mrvPy
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv2/mrvPy
    COMPONENT libraries )
