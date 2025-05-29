set(HEADERS
    player-qtquick.h)

set(SOURCE
    player-qtquick.cpp
    main.cpp)

if(TLRENDER_QT6)
    qt6_add_resources(SOURCE player-qtquick.qrc)
elseif(TLRENDER_QT5)
    qt5_add_resources(SOURCE player-qtquick.qrc)
endif()

add_executable(player-qtquick ${SOURCE} ${HEADERS})
target_link_libraries(player-qtquick tlQtQuick tlBaseApp)
set_target_properties(player-qtquick PROPERTIES FOLDER examples)
