find_package(Libssh2 REQUIRED)

set(source main.cpp)

add_executable(Libssh2Test ${header} ${source})
target_link_libraries(Libssh2Test Libssh2::libssh2_static)

add_test(Libssh2Test Libssh2Test)
