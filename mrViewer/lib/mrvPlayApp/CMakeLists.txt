set(HEADERS
    App.h
    mrvColorModel.h
    mrvFilesModel.h
    mrvOpenSeparateAudioDialog.h
    mrvSettingsObject.h
  )
  set(SOURCES
    App.cpp
    mrvColorModel.cpp
    mrvFilesModel.cpp
    mrvOpenSeparateAudioDialog.cpp
    mrvSettingsObject.cpp
  )

add_library(mrvPlayApp ${SOURCES} ${HEADERS})
target_link_libraries(mrvPlayApp tlApp mrvGL mrvFl )
target_include_directories(mrvPlayApp PUBLIC ${INCLUDES} )
set_target_properties(mrvPlayApp PROPERTIES FOLDER lib)
set_target_properties(mrvPlayApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvPlayApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv/mrvPlayApp)
