
if (USE_SYSTEM_LIBS)
    find_package(FFmpeg)
    find_package(SvtAV1)
    set(SvtAV1_DEP )
endif()

if (NOT SvtAV1_FOUND AND NOT FFmpeg_FOUND)
    include(ExternalProject)

    set(SvtAV1_GIT_REPOSITORY "https://gitlab.com/AOMediaCodec/SVT-AV1.git")
    set(SvtAV1_GIT_TAG "v4.0.1")

    # This tries to use NASM or GAS for compilation
    set(SvtAV1_ARGS
	${TLRENDER_EXTERNAL_ARGS}
	-DBUILD_APPS=OFF
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON
	-DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_PREFIX}/include
	-DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_PREFIX}/lib
	-DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_PREFIX}/bin)


    set(SvtAV1_DEPENDENCIES )
    if(NOT WIN32)
	if(SYSTEM_PROCESSOR_LC MATCHES ".*amd64.*")
	    if(UNIX)
		list(APPEND SvtAV1_DEPENDENCIES ${NASM_DEP})
		if (EXISTS "${CMAKE_INSTALL_PREFIX}/bin/nasm")
		    list(APPEND SvtAV1_ARGS -DCMAKE_ASM_NASM_COMPILER=${CMAKE_INSTALL_PREFIX}/bin/nasm)
		else()
		    find_program(NASM_EXECUTABLE NAMES nasm.exe nasm)
		    list(APPEND SvtAV1_ARGS
			-DCMAKE_ASM_NASM_COMPILER=${NASM_EXECUTABLE})
		endif()
	    endif()
	endif()
    endif()

    message(STATUS "SvtAV1 DEPENDENCIES=${SvtAV1_DEPENDENCIES}")

    ExternalProject_Add(
	SvtAV1
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/SvtAV1

	GIT_REPOSITORY ${SvtAV1_GIT_REPOSITORY}
	GIT_TAG ${SvtAV1_GIT_TAG}
	
	DEPENDS ${SvtAV1_DEPENDENCIES}
	
	LIST_SEPARATOR |
	CMAKE_ARGS ${SvtAV1_ARGS}
    )

    set(SvtAV1_DEP SvtAV1)

endif()
