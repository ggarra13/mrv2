add_executable(player player.h player.cpp)
target_link_libraries(player tlUIApp tlTimelineUI)
set_target_properties(player PROPERTIES FOLDER examples)
