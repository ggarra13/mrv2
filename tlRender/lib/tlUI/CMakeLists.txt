set(HEADERS
#    Action.h
#    ButtonGroup.h
#    Bellows.h
#    CheckBox.h
#    ComboBox.h
#    ColorPopup.h
#    ColorSwatch.h
#    ColorWidget.h
#    Divider.h
#    DoubleEdit.h
#    DoubleEditSlider.h
#    DoubleModel.h
#    DoubleSlider.h
    DrawUtil.h
#    DrivesModel.h
    Event.h
#    FileBrowser.h
#    FileEdit.h
#    FloatEdit.h
#    FloatEditSlider.h
#    FloatModel.h
#    FloatSlider.h
    GridLayout.h
#    GroupBox.h
    IButton.h
    IClipboard.h
    IDialog.h
    IMenuPopup.h
    IPopup.h
    IWidget.h
    IWidgetInline.h
    IWidgetOptions.h
    IWidgetPopup.h
    IWindow.h
    IconLibrary.h
#    Icon.h
#    IncButtons.h
    Init.h
#    IntEdit.h
#    IntEditSlider.h
#    IntModel.h
#    IntSlider.h
    Label.h
    LayoutUtil.h
#    LineEdit.h
#    ListButton.h
#    ListWidget.h
#    MDICanvas.h
#    MDIWidget.h
#    Menu.h
#    MenuBar.h
#    MessageDialog.h
#    OverlayLayout.h
#    PieChart.h
#    PushButton.h
#    RecentFilesModel.h
    RowLayout.h
    ScrollArea.h
    ScrollWidget.h
#    Spacer.h
    ScrollBar.h
#    SearchBox.h
#    Splitter.h
#    StackLayout.h
    Style.h
    StyleInline.h
#    TabBar.h
    ThumbnailSystem.h
#    TimeEdit.h
#    TimeLabel.h
#    ToolButton.h
    ToolTip.h
)
set(HEADERS_PRIVATE
    #FileBrowserPrivate.h
)

set(SOURCE
   # Action.cpp
   # ButtonGroup.cpp
   # Bellows.cpp
   # CheckBox.cpp
   # ComboBox.cpp
   # ColorPopup.cpp
   # ColorSwatch.cpp
   # ColorWidget.cpp
   # Divider.cpp
   # DoubleEdit.cpp
   # DoubleEditSlider.cpp
   # DoubleModel.cpp
   # DoubleSlider.cpp
    DrawUtil.cpp
   # DrivesModel.cpp
    Event.cpp
#    FileBrowser.cpp
#    FileEdit.cpp

    # FileBrowserButton.cpp
    # FileBrowserDirectoryWidget.cpp
    # FileBrowserPathsWidget.cpp
    # FileBrowserSystem.cpp
    # FileBrowserWidget.cpp

    #  FloatEdit.cpp
  #  FloatEditSlider.cpp
  #  FloatModel.cpp
  #  FloatSlider.cpp
    GridLayout.cpp
  #  GroupBox.cpp
    IButton.cpp
    IClipboard.cpp
    IDialog.cpp
    IMenuPopup.cpp
    IPopup.cpp
    IWidget.cpp
    IWidgetOptions.cpp
    IWidgetPopup.cpp
    IWindow.cpp
    IconLibrary.cpp
  #  Icon.cpp
  #  IncButtons.cpp
    Init.cpp
  #  IntEdit.cpp
  #  IntEditSlider.cpp
  #  IntModel.cpp
  #  IntSlider.cpp
    Label.cpp
    LayoutUtil.cpp
  #  LineEdit.cpp
  #  ListButton.cpp
  #  ListWidget.cpp
  #  MDICanvas.cpp
  #  MDIWidget.cpp
  #  Menu.cpp
  #  MenuBar.cpp
  # MessageDialog.cpp
  #  OverlayLayout.cpp
  #  PieChart.cpp
  #  PushButton.cpp
  #  RecentFilesModel.cpp
    RowLayout.cpp
    ScrollArea.cpp
    ScrollWidget.cpp
  #  SearchBox.cpp
  #  Spacer.cpp
    ScrollBar.cpp
  #  Splitter.cpp
  #  StackLayout.cpp
    Style.cpp
  #  TabBar.cpp
    ThumbnailSystem.cpp
  #  TimeEdit.cpp
  #  TimeLabel.cpp
  #  ToolButton.cpp
    ToolTip.cpp
)

set(LIBRARIES_PRIVATE)

add_library(tlUI ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlUI PUBLIC tlTimelineGL PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlUI PROPERTIES FOLDER lib)
set_target_properties(tlUI PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlUI
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlUI)
