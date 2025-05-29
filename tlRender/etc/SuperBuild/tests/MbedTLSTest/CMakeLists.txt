find_package(MbedTLS REQUIRED)

set(source main.cpp)

add_executable(MbedTLSTest ${header} ${source})
target_link_libraries(MbedTLSTest MbedTLS::mbedtls)

add_test(MbedTLSTest MbedTLSTest)
