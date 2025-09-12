set(HEADERS
    DrawUtil.h
    Event.h
    GridLayout.h
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
    Init.h
    Label.h
    LayoutUtil.h
    RowLayout.h
    ScrollArea.h
    ScrollWidget.h
    ScrollBar.h
    Style.h
    StyleInline.h
    ToolTip.h
)
set(HEADERS_PRIVATE
)

set(SOURCE
    DrawUtil.cpp
    Event.cpp
    GridLayout.cpp
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
    Init.cpp
    Label.cpp
    LayoutUtil.cpp
    RowLayout.cpp
    ScrollArea.cpp
    ScrollWidget.cpp
    ScrollBar.cpp
    Style.cpp
    ToolTip.cpp
)

set(LIBRARIES_PRIVATE)

add_library(tlUI ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
if(TLRENDER_GLFW OR TLRENDER_GL)
    list(APPEND LIBRARIES tlTimelineGL)
endif()
if(TLRENDER_VK)
    list(APPEND LIBRARIES tlTimelineVk)
endif()
target_link_libraries(tlUI PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlUI PROPERTIES FOLDER lib)
set_target_properties(tlUI PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlUI
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlUI)
