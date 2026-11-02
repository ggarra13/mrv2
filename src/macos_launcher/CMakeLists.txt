

set(HEADERS
)
set(SOURCES
    main.cpp)


add_executable(macos_launcher ${SOURCES} ${HEADERS})

install(TARGETS macos_launcher
    RUNTIME DESTINATION bin COMPONENT application
    LIBRARY DESTINATION lib COMPONENT libraries
    ARCHIVE DESTINATION lib COMPONENT libraries )
