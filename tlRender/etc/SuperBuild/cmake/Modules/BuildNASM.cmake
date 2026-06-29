include(ExternalProject)

if (USE_SYSTEM_LIBS)
    find_program(NASM_EXECUTABLE nasm NAMES nasm.exe nasm)
    set(NASM_DEP )
endif()

if (NOT NASM_EXECUTABLE)
    set(NASM_GIT_TAG nasm-3.02rc3)

    ExternalProject_Add(
	NASM
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/NASM
	GIT_REPOSITORY https://github.com/netwide-assembler/nasm
	GIT_TAG ${NASM_GIT_TAG}
	CONFIGURE_COMMAND sh autogen.sh && sh configure
	BUILD_COMMAND make
	INSTALL_COMMAND cp nasm ${CMAKE_INSTALL_PREFIX}/bin
	BUILD_IN_SOURCE 1)

    set(NASM_EXECUTABLE ${CMAKE_INSTALL_PREFIX}/bin/nasm)
    set(NASM_DEP NASM)
endif()
