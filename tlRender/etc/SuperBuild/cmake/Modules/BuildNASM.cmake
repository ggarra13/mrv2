include(ExternalProject)

ExternalProject_Add(
    NASM
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/NASM
    URL https://github.com/netwide-assembler/nasm/archive/refs/tags/nasm-2.16.01.tar.gz
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/NASM-patch/configure
        ${CMAKE_CURRENT_BINARY_DIR}/NASM/src/NASM/configure
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/NASM-patch/Makefile.in
        ${CMAKE_CURRENT_BINARY_DIR}/NASM/src/NASM/Makefile.in
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/NASM-patch/config/config.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/NASM/src/NASM/config/config.h.in
    CONFIGURE_COMMAND sh configure --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_IN_SOURCE 1)
