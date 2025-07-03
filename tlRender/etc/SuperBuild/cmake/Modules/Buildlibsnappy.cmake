include(ExternalProject)

#
# Original with Microsoft and Google conflicts.
#
# set(libsnappy_REPO "https://github.com/google/snappy.git")
# set(libsnappy_TAG 6af9287fbdb913f0794d0148c6aa43b58e63c8e3)

#
# My copy of it.
#
set(libsnappy_REPO "https://github.com/ggarra13/libsnappy.git")
set(libsnappy_TAG 6af9287fbdb913f0794d0148c6aa43b58e63c8e36)


set(libsnappy_ARGS ${TLRENDER_EXTERNAL_ARGS})

if(APPLE)
    set(libsnappy_CXX_COMPILER $ENV{NATIVE_COMPILER_NAME}) #not yet used
    set(libsnappy_C_COMPILER $ENV{NATIVE_COMPILER_NAME}) # not yet used
    set(libsnappy_CXX_FLAGS -fPIC ${CMAKE_CXX_FLAGS})
    set(libsnappy_C_FLAGS -fPIC ${CMAKE_C_FLAGS})
elseif (UNIX)
    set(libsnappy_CXX_COMPILER $ENV{NATIVE_COMPILER_NAME}) # not yet used
    set(libsnappy_C_COMPILER $ENV{NATIVE_COMPILER_NAME}) # not yet used
    set(libsnappy_CXX_FLAGS -fPIC ${CMAKE_CXX_FLAGS})
    set(libsnappy_C_FLAGS -fPIC ${CMAKE_C_FLAGS})
else()
    set(libsnappy_CXX_COMPILER $ENV{GENERIC_COMPILER})
    set(libsnappy_C_COMPILER $ENV{GENERIC_COMPILER})
    set(libsnappy_CXX_FLAGS "")
    set(libsnappy_C_FLAGS "")
endif()

message(STATUS "libsnappy_CXX_COMPILER=${libsnappy_CXX_COMPILER}")
message(STATUS "libsnappy_C_COMPILER=${libsnappy_C_COMPILER}")
message(STATUS "libsnappy CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
message(STATUS "libsnappy CMAKE_C_COMPILER=${CMAKE_C_COMPILER}")


list(APPEND libsnappy_ARGS
    -DBENCHMARK_ENABLE_INSTALL=OFF
    -DBENCHMARK_INSTALL_DOCS=OFF
    -DBENCHMARK_USE_BUNDLED_GTEST=OFF
    -DSNAPPY_BUILD_TESTS=OFF
    -DSNAPPY_BUILD_BENCHMARKS=OFF
    -DCMAKE_CXX_FLAGS="${libsnappy_CXX_FLAGS}"
    -DCMAKE_C_FLAGS="${libsnappy_C_FLAGS}"
)
if(WIN32)
    list(APPEND libsnappy_ARGS
	-DCMAKE_CXX_COMPILER=${libsnappy_CXX_COMPILER}
	-DCMAKE_C_COMPILER=${libsnappy_C_COMPILER}
    )
endif()

set(libsnappy_UPDATE_CMD git submodule update --init)


ExternalProject_Add(
    libsnappy
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libsnappy
    GIT_REPOSITORY ${libsnappy_REPO}
    GIT_TAG ${libsnappy_TAG}
    

    UPDATE_COMMAND ${libsnappy_UPDATE_CMD}

    STEP_TARGETS download
    # DOWNLOAD_COMMAND
    #     ${CMAKE_COMMAND} -E echo "Cloning repository..." &&
    #     git clone ${libsnappy_REPO} <SOURCE_DIR> &&
    #     ${CMAKE_COMMAND} -E echo "Git SHA256: " &&
    #     git -C <SOURCE_DIR> rev-parse HEAD &&
    #     ${CMAKE_COMMAND} -E echo "Git tags: " &&
    #     git -C <SOURCE_DIR> tag
	
    LIST_SEPARATOR |
    CMAKE_ARGS ${libsnappy_ARGS}
)

