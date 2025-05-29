set(HEADERS
    App.h
    AudioActions.h
    AudioTool.h
    ColorTool.h
    CompareActions.h
    DevicesTool.h
    DockTitleBar.h
    FileActions.h
    FilesTool.h
    FrameActions.h
    IToolWidget.h
    InfoTool.h
    InfoModel.h
    MainWindow.h
    MessagesTool.h
    OCIOModel.h
    OpenSeparateAudioDialog.h
    PlaybackActions.h
    RenderActions.h
    SecondaryWindow.h
    SettingsTool.h
    SettingsToolPrivate.h
    SystemLogTool.h
    TimelineActions.h
    ToolActions.h
    ViewActions.h
    ViewTool.h
    Viewport.h
    WindowActions.h)

set(SOURCE
    App.cpp
    AudioActions.cpp
    AudioTool.cpp
    ColorTool.cpp
    CompareActions.cpp
    DevicesTool.cpp
    DockTitleBar.cpp
    FileActions.cpp
    FilesTool.cpp
    FrameActions.cpp
    IToolWidget.cpp
    InfoTool.cpp
    InfoModel.cpp
    MainWindow.cpp
    MessagesTool.cpp
    OCIOModel.cpp
    OpenSeparateAudioDialog.cpp
    PlaybackActions.cpp
    RenderActions.cpp
    SecondaryWindow.cpp
    SettingsTool.cpp
    SystemLogTool.cpp
    TimelineActions.cpp
    ToolActions.cpp
    ViewActions.cpp
    ViewTool.cpp
    Viewport.cpp
    WindowActions.cpp)

if(TLRENDER_QT6)
    qt6_add_resources(SOURCE tlPlayQtApp.qrc)
elseif(TLRENDER_QT5)
    qt5_add_resources(SOURCE tlPlayQtApp.qrc)
endif()

add_library(tlPlayQtApp ${HEADERS} ${SOURCE})
target_link_libraries(tlPlayQtApp tlQtWidget tlPlay tlBaseApp)
set_target_properties(tlPlayQtApp PROPERTIES FOLDER lib)
set_target_properties(tlPlayQtApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlPlayQtApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlPlayQtApp)
