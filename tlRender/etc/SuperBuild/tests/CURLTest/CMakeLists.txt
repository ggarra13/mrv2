# \bug curl is already found?
if(NOT curl_FOUND)
    find_package(curl REQUIRED)
endif()

set(source main.cpp)

add_executable(curlTest ${header} ${source})
target_link_libraries(curlTest curl::curl)

add_test(curlTest curlTest)
