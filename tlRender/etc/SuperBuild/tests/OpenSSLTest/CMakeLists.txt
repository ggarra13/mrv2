find_package(OpenSSL REQUIRED)

set(source main.cpp)

add_executable(OpenSSLTest ${header} ${source})
target_link_libraries(OpenSSLTest OpenSSL)

add_test(OpenSSLTest OpenSSLTest)
