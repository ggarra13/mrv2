set(HEADERS
    App.h
    AudioModel.h
    ColorModel.h
    FilesModel.h
    Info.h
    Init.h
    OCIOModel.h
    RenderModel.h
    Settings.h
    SettingsInline.h
    Util.h
    ViewportModel.h)

set(SOURCE
    App.cpp
    AudioModel.cpp
    ColorModel.cpp
    FilesModel.cpp
    Info.cpp
    Init.cpp
    OCIOModel.cpp
    RenderModel.cpp
    Settings.cpp
    Util.cpp
    ViewportModel.cpp)

add_library(tlPlay ${HEADERS} ${SOURCE})
target_link_libraries(tlPlay tlDevice)
set_target_properties(tlPlay PROPERTIES FOLDER lib)
set_target_properties(tlPlay PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlPlay
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlPlay)
