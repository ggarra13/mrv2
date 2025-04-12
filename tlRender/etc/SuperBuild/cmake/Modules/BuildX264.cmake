include(ExternalProject)


#set(X264_TAG master) # live on the cutting-edge!
set(X264_TAG stable) # live on the cutting-edge!


include(ProcessorCount)
ProcessorCount(NPROCS)

set(X264_CFLAGS)
set(X264_CXXFLAGS)
set(X264_OBJCFLAGS)
set(X264_LDFLAGS)
set(X264_DEPENDENCIES NASM)

if (WIN32)
    include(functions/Msys2)
    # Convert path for MSYS2 properly
    convert_path_for_msys2("${CMAKE_INSTALL_PREFIX}" INSTALL_PREFIX)
    set(X264_DEPENDENCIES)
else()
    set(INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
endif()


if(APPLE)
    set(X264_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(X264_C_FLAGS "${CMAKE_C_FLAGS}")
    if (CMAKE_OSX_DEPLOYMENT_TARGET)
        list(APPEND X264_CXX_FLAGS "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
        list(APPEND X264_C_FLAGS "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()
endif()

set(X264_CONFIGURE_ARGS
    --prefix=${INSTALL_PREFIX}
    --enable-pic
    --enable-shared
    --disable-static
    "CFLAGS=${X264_C_FLAGS}"
    "CXXFLAGS=${X264_CXX_FLAGS}"
)

set(X264_CONFIGURE_COMMAND ./configure ${X264_CONFIGURE_ARGS})
set(X264_BUILD_COMMAND make -j ${NPROCS})
set(X264_INSTALL_COMMAND make install)

if(WIN32)
    # Build X264 with MSYS2 on Windows.
    find_package(Msys REQUIRED)
    set(X264_MSYS2
        ${MSYS_CMD}
        -use-full-path
        -defterm
        -no-start
        -here)

    # Properly format VPX_CONFIGURE_ARGS
    list(JOIN X264_CONFIGURE_ARGS " " X264_CONFIGURE_ARGS_STR)

    set(X264_CONFIGURE_COMMAND ${X264_MSYS2} -c "pacman -S make diffutils nasm --noconfirm && CC=cl ./configure ${X264_CONFIGURE_ARGS_STR}")
    set(X264_BUILD_COMMAND ${X264_MSYS2} -c "make -j ${NPROCS}")
    set(X264_INSTALL_COMMAND ${X264_MSYS2} -c "make install && mv ${INSTALL_PREFIX}/lib/libx264.dll.lib ${INSTALL_PREFIX}/lib/libx264.lib")
endif()

ExternalProject_Add(
    X264
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/X264
    DEPENDS ${X264_DEPENDENCIES}
    GIT_REPOSITORY "https://code.videolan.org/videolan/x264.git"
    GIT_TAG ${X264_TAG}
    
    CONFIGURE_COMMAND ${X264_CONFIGURE_COMMAND}
    BUILD_COMMAND ${X264_BUILD_COMMAND}
    INSTALL_COMMAND ${X264_INSTALL_COMMAND}
    BUILD_IN_SOURCE 1
)
