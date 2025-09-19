include(ExternalProject)

include(GNUInstallDirs)

#set(SvtAV1_TAG v3.0.0)  # 3.0+ branch not ocmpatible with FFmpeg 7.0.1
set(SvtAV1_TAG v2.3.0)

set(SvtAV1_ARGS ${TLRENDER_EXTERNAL_ARGS})
list(APPEND SvtAV1_ARGS
    -DCMAKE_INSTALL_LIBDIR=lib
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5  # uses old CMake files
)

if (WIN32)
    if ($ENV{ARCH} MATCHES ".*amd64.*")
	list(APPEND SvtAV1_ARGS
	    -DENABLE_NASM=ON)
    else()
	list(APPEND SvtAV1_ARGS
	    -DENABLE_NASM=OFF)
    endif()
else()
    list(APPEND SvtAV1_ARGS
	-DENABLE_NASM=ON)
endif()


set(SvtAV1_DEPS )
if(NOT WIN32)
    set(SvtAV1_DEPS NASM)
else()
    include(functions/Msys2)
    set(SvtAV1_MSYS2 ${MRV2_MSYS_CMD})
endif()

ExternalProject_Add(
    SvtAV1
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/SvtAV1

    GIT_REPOSITORY "https://gitlab.com/AOMediaCodec/SVT-AV1.git"
    GIT_TAG ${SvtAV1_TAG}
    
    DEPENDS ${SvtAV1_DEPS} ${Gettext_DEP}

    PATCH_COMMAND ${SvtAV1_PATCH}
    
    LIST_SEPARATOR |
    CMAKE_ARGS ${SvtAV1_ARGS}
)
