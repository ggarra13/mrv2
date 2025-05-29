find_package(libjpeg-turbo REQUIRED)

set(source main.cpp)

add_executable(libjpeg-turboTest ${header} ${source})
target_link_libraries(libjpeg-turboTest libjpeg-turbo::turbojpeg-static)

add_test(libjpeg-turboTest libjpeg-turboTest)
