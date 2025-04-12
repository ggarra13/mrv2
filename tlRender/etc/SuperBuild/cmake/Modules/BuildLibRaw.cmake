include(ExternalProject)


set(LibRaw_URL "https://www.libraw.org/data/LibRaw-0.21.1.tar.gz")
set(LibRaw_TAG )

set(LibRaw_cmake_URL "https://github.com/LibRaw/LibRaw-cmake")
set(LibRaw_cmake_TAG 35127051d127f1e1e3dc0ec3a9f59fa48a244ddd)

ExternalProject_Add(
    LibRaw_cmake
    GIT_REPOSITORY ${LibRaw_cmake_URL}
    GIT_TAG ${LibRaw_cmake_TAG}
    BUILD_IN_SOURCE 0
    BUILD_ALWAYS 0
    UPDATE_COMMAND ""
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/LibRaw_cmake
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

set(LibRaw_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
if(APPLE)
    set(LibRaw_CXX_FLAGS "-Wno-register ${CMAKE_CXX_FLAGS}")
endif()

set(LibRaw_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DBUILD_SHARED_LIBS=ON
    -DBUILD_STATIC_LIBS=OFF
    -DENABLE_OPENMP=ON
    -DENABLE_JASPER=ON
    -DENABLE_LCMS=ON
    -DENABLE_X3FTOOLS=ON
    -DENABLE_EXAMPLES=OFF
    -DBUILD_TESTING=OFF
    -DCMAKE_CXX_FLAGS=${LibRaw_CXX_FLAGS}
)

#
# This copies the .cmake files from Libraw-cmake.  Two files need patching thou.
#
set(LibRaw_PATCH
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/LibRaw-patch/CMakeLists.txt <SOURCE_DIR>
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_CURRENT_BINARY_DIR}/LibRaw_cmake/src/LibRaw_cmake/cmake <SOURCE_DIR>/cmake
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/LibRaw-patch/FindLCMS2.cmake <SOURCE_DIR>/cmake/modules
)

set(LibRaw_DEPS LibRaw_cmake jasper LCMS2 ZLIB)
if(TLRENDER_JPEG)
    list(APPEND LibRaw_DEPS libjpeg-turbo)
endif()
    
ExternalProject_Add(
     LibRaw
     PREFIX ${CMAKE_CURRENT_BINARY_DIR}/LibRaw
     URL ${LibRaw_URL}
     DEPENDS ${LibRaw_DEPS}
     PATCH_COMMAND ${LibRaw_PATCH}
     LIST_SEPARATOR |
     CMAKE_ARGS ${LibRaw_ARGS}
)
