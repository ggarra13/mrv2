find_package(ZLIB REQUIRED)
find_package(minizip REQUIRED)
find_package(OpenColorIO REQUIRED)

set(source main.cpp)

add_executable(OpenColorIOTest ${header} ${source})
target_link_libraries(OpenColorIOTest OpenColorIO::OpenColorIO MINIZIP::minizip ZLIB)

add_test(OpenColorIOTest OpenColorIOTest)
