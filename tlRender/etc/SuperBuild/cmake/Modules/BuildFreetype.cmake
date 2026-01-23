include(ExternalProject)

set(Freetype_GIT_REPOSITORY "https://github.com/freetype/freetype.git")
set(Freetype_GIT_TAG "VER-2-13-3")

set(Freetype_DEPENDENCIES PNG ZLIB)
message(STATUS "Freetype_DEPENDENCIES=${Freetype_DEPENDENCIES}")

set(Freetype_DISABLE_HARFBUZZ OFF)
if (APPLE)
    set(Freetype_DISABLE_HARFBUZZ ON)
endif()


set(Freetype_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DFT_REQUIRE_ZLIB=ON
    -DFT_REQUIRE_PNG=ON
    -DFT_DISABLE_BZIP2=ON
    -DFT_DISABLE_HARFBUZZ=${Freetype_DISABLE_HARFBUZZ}
    -DFT_DISABLE_BROTLI=ON
)

ExternalProject_Add(
    Freetype
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/Freetype
    DEPENDS ${Freetype_DEPENDENCIES}
    GIT_REPOSITORY ${Freetype_GIT_REPOSITORY}
    GIT_TAG ${Freetype_GIT_TAG}
    
    LIST_SEPARATOR |
    CMAKE_ARGS ${Freetype_ARGS})
