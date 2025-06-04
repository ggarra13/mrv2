set(HEADERS
    App.h
    FloatEditSlider.h
    IntEditSlider.h
    MainWindow.h)

set(SOURCE
    App.cpp
    FloatEditSlider.cpp
    IntEditSlider.cpp
    MainWindow.cpp
    main.cpp)

add_executable(widgets-qtwidget ${HEADERS} ${SOURCE})
target_link_libraries(widgets-qtwidget tlQtWidget)
set_target_properties(widgets-qtwidget PROPERTIES FOLDER examples)
