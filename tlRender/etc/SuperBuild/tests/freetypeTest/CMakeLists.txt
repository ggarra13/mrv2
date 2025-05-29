find_package(Freetype REQUIRED)

set(source main.cpp)

add_executable(FreetypeTest ${header} ${source})
target_link_libraries(FreetypeTest Freetype::Freetype ZLIB)

add_test(FreetypeTest FreetypeTest)
