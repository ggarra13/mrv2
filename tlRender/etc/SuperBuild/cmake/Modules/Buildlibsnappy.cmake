include(ExternalProject)


set(libsnappy_REPO "https://github.com/google/snappy.git")
set(libsnappy_TAG 1.2.2)


set(libsnappy_ARGS ${TLRENDER_EXTERNAL_ARGS})


if (UNIX)
    set(libsnappy_CXX_FLAGS -fPIC ${CMAKE_CXX_FLAGS})
    set(libsnappy_C_FLAGS -fPIC ${CMAKE_C_FLAGS})
else()
    set(libsnappy_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    set(libsnappy_C_FLAGS ${CMAKE_C_FLAGS})
endif()

list(APPEND libsnappy_ARGS
    -DBENCHMARK_ENABLE_INSTALL=OFF
    -DBENCHMARK_INSTALL_DOCS=OFF
    -DBENCHMARK_USE_BUNDLED_GTEST=OFF
    -DSNAPPY_BUILD_TESTS=OFF
    -DSNAPPY_BUILD_BENCHMARKS=OFF
    -DCMAKE_CXX_FLAGS="${libsnappy_CXX_FLAGS}"
    -DCMAKE_C_FLAGS="${libsnappy_C_FLAGS}"
)

set(libsnappy_UPDATE_CMD git submodule update --init)


ExternalProject_Add(
    libsnappy
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libsnappy
    GIT_REPOSITORY ${libsnappy_REPO}
    GIT_TAG ${libsnappy_TAG}
    

    UPDATE_COMMAND ${libsnappy_UPDATE_CMD}
    
    LIST_SEPARATOR |
    CMAKE_ARGS ${libsnappy_ARGS}
)

