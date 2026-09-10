include(ExternalProject)

ExternalProject_Add(
    nv-codec-headers
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/../../../deps/nv-codec-headers
    # Alternative repo (older)
    # URL https://github.com/FFmpeg/nv-codec-headers/archive/refs/tags/n12.2.72.0.tar.gz
    GIT_REPOSITORY "https://git.videolan.org/git/ffmpeg/nv-codec-headers.git"
    GIT_TAG n13.0.19.1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND make install PREFIX=${CMAKE_INSTALL_PREFIX}
    BUILD_IN_SOURCE 1)
