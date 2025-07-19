# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
)

set(SOURCES
    Annotations.cpp
    Arrow.cpp
    Audio.cpp
    Circle.cpp
    Clear.cpp
    ColorArea.cpp
    Color.cpp
    CompareA.cpp
    CompareB.cpp
    Compare.cpp
    CompareDifference.cpp
    CompareHorizontal.cpp
    CompareOverlay.cpp
    CompareTile.cpp
    CompareVertical.cpp
    CompareWipe.cpp
    Devices.cpp
    Documents.cpp
    Draw.cpp
    Empty.cpp
    EnvironmentMap.cpp
    Erase.cpp
    FavoritesButton.cpp
    FavoritesFolders.cpp
    FileBrowser.cpp
    FileCloseAll.cpp
    FileClose.cpp
    FileOpen.cpp
    FileOpenSeparateAudio.cpp
    Files.cpp
    FilledCircle.cpp
    FilledPolygon.cpp
    FilledRectangle.cpp
    Filter.cpp
    Flip.cpp
    FrameNext.cpp
    FramePrev.cpp
    HardBrush.cpp
    Histogram.cpp
    Home.cpp
    Info.cpp
    Logs.cpp
    LoopMode.cpp
    Messages.cpp
    mrv2.cpp
    Music.cpp
    Mute.cpp
    NDI.cpp
    Network.cpp
    NewFolder.cpp
    Next.cpp
    OutlineCircle.cpp
    OutlinePolygon.cpp
    OutlineRectangle.cpp
    Playlist.cpp
    Polygon.cpp
    Prev.cpp
    Python.cpp
    Rectangle.cpp
    Redo.cpp
    RefreshDir.cpp
    Reset.cpp
    Save.cpp
    Scrub.cpp
    Selection.cpp
    Settings.cpp
    SoftBrush.cpp
    Stereo3D.cpp
    StopMode.cpp
    SwingMode.cpp
    TemporaryButton.cpp
    Text.cpp
    Tracks.cpp
    TracksFromA.cpp
    Trashcan.cpp
    Undo.cpp
    UpFolder.cpp
    USD.cpp
    Vectorscope.cpp
    Volume.cpp
    WindowFullScreen.cpp
    WindowSecondary.cpp
)


add_library(mrvIcons ${SOURCES} ${HEADERS})
target_link_libraries(mrvIcons PUBLIC ${LIBRARIES} )
set_target_properties(mrvIcons PROPERTIES FOLDER lib)

install(TARGETS mrvIcons
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)
