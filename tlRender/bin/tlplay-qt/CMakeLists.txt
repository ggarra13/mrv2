add_executable(tlplay-qt WIN32 main.cpp)
target_link_libraries(tlplay-qt tlPlayQtApp)
set_target_properties(tlplay-qt PROPERTIES FOLDER bin)

install(
    TARGETS tlplay-qt
    RUNTIME DESTINATION bin)

if(WIN32)
    add_executable(tlplay-qt. main.cpp)
    target_link_libraries(tlplay-qt. tlPlayQtApp)    
    set_target_properties(
        tlplay-qt. PROPERTIES
        SUFFIX com
        FOLDER bin)
    install(
        TARGETS tlplay-qt.
        RUNTIME DESTINATION bin)
endif()

