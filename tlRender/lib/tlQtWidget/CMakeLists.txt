set(HEADERS
    BellowsWidget.h
    ColorDialog.h
    ColorSwatch.h
    ColorWidget.h
    FileBrowserSystem.h
    FileWidget.h
    FloatEditSlider.h
    Init.h
    IntEditSlider.h
    SearchWidget.h
    Separator.h
    Spacer.h
    SpeedLabel.h
    Style.h
    TimeLabel.h
    TimeSpinBox.h
    TimelineViewport.h
    TimelineWidget.h
    Util.h)
set(PRIVATE_HEADERS
    BellowsPrivate.h)

set(SOURCE
    BellowsPrivate.cpp
    BellowsWidget.cpp
    ColorDialog.cpp
    ColorSwatch.cpp
    ColorWidget.cpp
    FileBrowserSystem.cpp
    FileWidget.cpp
    FloatEditSlider.cpp
    Init.cpp
    IntEditSlider.cpp
    SearchWidget.cpp
    Separator.cpp
    Spacer.cpp
    SpeedLabel.cpp
    Style.cpp
    TimeLabel.cpp
    TimeSpinBox.cpp
    TimelineViewport.cpp
    TimelineWidget.cpp
    Util.cpp)

if(TLRENDER_QT6)
    qt6_add_resources(SOURCE tlQtWidget.qrc)
elseif(TLRENDER_QT5)
    qt5_add_resources(SOURCE tlQtWidget.qrc)
endif()

set(LIBRARIES tlQt tlTimelineUI)
if(TLRENDER_QT6)
    list(APPEND LIBRARIES Qt6::Widgets Qt6::OpenGLWidgets Qt6::Svg)
elseif(TLRENDER_QT5)
    list(APPEND LIBRARIES Qt5::Widgets Qt5::Svg)
endif()

add_library(tlQtWidget ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlQtWidget ${LIBRARIES})
set_target_properties(tlQtWidget PROPERTIES FOLDER lib)
set_target_properties(tlQtWidget PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlQtWidget
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlQtWidget)
