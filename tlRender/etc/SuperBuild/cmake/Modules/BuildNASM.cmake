include(ExternalProject)

set(NAMS_GIT_TAG nasm-3.02rc3)

ExternalProject_Add(
    NASM
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/NASM
    GIT_REPOSITORY https://github.com/netwide-assembler/nasm
    GIT_TAG ${NASM_GIT_TAG}
    CONFIGURE_COMMAND sh autogen.sh && sh configure
    BUILD_COMMAND make
    INSTALL_COMMAND cp nasm ${CMAKE_INSTALL_PREFIX}/bin
    BUILD_IN_SOURCE 1)
