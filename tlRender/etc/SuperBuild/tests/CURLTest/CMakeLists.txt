# \bug CURL is already found?
if(NOT CURL_FOUND)
    find_package(CURL REQUIRED)
endif()

set(source main.cpp)

add_executable(CURLTest ${header} ${source})
target_link_libraries(CURLTest CURL::libcurl_static)

add_test(CURLTest CURLTest)
