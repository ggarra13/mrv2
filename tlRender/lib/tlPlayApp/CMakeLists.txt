set(HEADERS
    App.h
    AudioActions.h
    AudioMenu.h
    AudioPopup.h
    AudioTool.h
    ColorTool.h
    CompareActions.h
    CompareMenu.h
    CompareToolBar.h
    DevicesTool.h
    FileActions.h
    FileMenu.h
    FileToolBar.h
    FilesTool.h
    FrameActions.h
    FrameMenu.h
    IToolWidget.h
    InfoTool.h
    MainWindow.h
    MessagesTool.h
    PlaybackActions.h
    PlaybackMenu.h
    RenderActions.h
    RenderMenu.h
    SeparateAudioDialog.h
    SpeedPopup.h
    SecondaryWindow.h
    SettingsTool.h
    SettingsToolPrivate.h
    Style.h
    SystemLogTool.h
    TimelineActions.h
    TimelineMenu.h
    Tools.h
    ToolsActions.h
    ToolsMenu.h
    ToolsToolBar.h
    ToolsWidget.h
    ViewActions.h
    ViewMenu.h
    ViewToolBar.h
    ViewTool.h
    Viewport.h
    WindowActions.h
    WindowMenu.h
    WindowToolBar.h)
set(HEADERS_PRIVATE
    ColorToolPrivate.h
    FilesToolPrivate.h
    SeparateAudioPrivate.h
    ViewToolPrivate.h)

set(SOURCE
    App.cpp
    AudioActions.cpp
    AudioMenu.cpp
    AudioPopup.cpp
    AudioTool.cpp
    ColorTool.cpp
    CompareActions.cpp
    CompareMenu.cpp
    CompareToolBar.cpp
    DevicesTool.cpp
    FileActions.cpp
    FileButton.cpp
    FileMenu.cpp
    FileToolBar.cpp
    FilesTool.cpp
    FrameActions.cpp
    FrameMenu.cpp
    IToolWidget.cpp
    InfoTool.cpp
    MainWindow.cpp
    MessagesTool.cpp
    PlaybackActions.cpp
    PlaybackMenu.cpp
    RenderActions.cpp
    RenderMenu.cpp
    SeparateAudioDialog.cpp
    SeparateAudioWidget.cpp
    SpeedPopup.cpp
    SecondaryWindow.cpp
    SettingsTool.cpp
    Style.cpp
    SystemLogTool.cpp
    TimelineActions.cpp
    TimelineMenu.cpp
    Tools.cpp
    ToolsActions.cpp
    ToolsMenu.cpp
    ToolsToolBar.cpp
    ToolsWidget.cpp
    ViewActions.cpp
    ViewMenu.cpp
    ViewToolBar.cpp
    ViewTool.cpp
    Viewport.cpp
    WindowActions.cpp
    WindowMenu.cpp
    WindowToolBar.cpp)

add_library(tlPlayApp ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlPlayApp tlPlay tlTimelineUI tlUIApp)
set_target_properties(tlPlayApp PROPERTIES FOLDER lib)
set_target_properties(tlPlayApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlPlayApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlPlayApp)
