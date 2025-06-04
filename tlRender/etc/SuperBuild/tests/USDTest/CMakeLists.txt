find_package(pxr REQUIRED)

set(source main.cpp)

add_executable(USDTest ${header} ${source})
target_link_libraries(USDTest tf usd usdGeom usdImagingGL usdAppUtils hd)

add_test(USDTest USDTest)
