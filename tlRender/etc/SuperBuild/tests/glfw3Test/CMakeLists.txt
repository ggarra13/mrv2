find_package(glfw3 REQUIRED)

set(source main.cpp)

add_executable(glfw3Test ${header} ${source})
target_link_libraries(glfw3Test glfw)

add_test(glfw3Test glfw3Test)
