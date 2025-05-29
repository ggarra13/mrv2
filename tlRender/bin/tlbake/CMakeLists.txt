add_executable(tlbake main.cpp)
target_link_libraries(tlbake tlBakeApp)
set_target_properties(tlbake PROPERTIES FOLDER bin)

install(
    TARGETS tlbake
    RUNTIME DESTINATION bin)
