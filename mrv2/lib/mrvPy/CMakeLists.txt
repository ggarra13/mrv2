# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    Cmds.h
    CmdsAux.h
    PyStdErrOutRedirect.h
)
set(SOURCES
    Annotations.cpp
    Cmds.cpp
    Enums.cpp
    IO.cpp
    FLTKRedirectOutput.cpp
    FileItem.cpp
    FilesModel.cpp
    FilePath.cpp
    Image.cpp
    Math.cpp
    Media.cpp
    OTIO.cpp
    Playlist.cpp
    Plugin.cpp
    Session.cpp
    Settings.cpp
    Timeline.cpp
    USD.cpp
)

add_library(mrvPy ${SOURCES} ${HEADERS})
files_to_absolute_paths()


set(LIBRARIES mrvApp)

target_link_libraries(mrvPy PUBLIC ${LIBRARIES} )
target_include_directories(mrvPy PUBLIC ${INCLUDES} )
set_target_properties(mrvPy PROPERTIES FOLDER lib)



install(TARGETS mrvPy
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries )
