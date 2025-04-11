find_package(OTIO REQUIRED)

set(source main.cpp)

add_executable(OTIOTest ${header} ${source})
target_link_libraries(OTIOTest OTIO)

add_test(OTIOTest OTIOTest)
