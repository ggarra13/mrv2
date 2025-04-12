include(ExternalProject)


# set(LIBVPX_TAG main) # live on the cutting-edge!

set(VPX_TAG v1.15.0)


include(ProcessorCount)
ProcessorCount(NPROCS)

set(VPX_MSYS2)
if(WIN32)
    # Build VPX with MSYS2 on Windows.
    find_package(Msys REQUIRED)
    set(VPX_MSYS2
        ${MSYS_CMD}
        -use-full-path
        -defterm
        -no-start
        -here)
endif()

set(VPX_CFLAGS)
set(VPX_CXXFLAGS)
set(VPX_OBJCFLAGS)
set(VPX_LDFLAGS)
set(VPX_TARGET)
set(VPX_DEPENDS)
set(VPX_ENV)
if (UNIX)
    # We modify PATH so nasm is found.  On windows, it is already in Msys2
    # path.
    set(VPX_DEPENDS NASM)
    set(VPX_ENV PATH="${CMAKE_INSTALL_PREFIX}/bin:$ENV{PATH}")
    set(INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
else()
    include(functions/Msys2)
    
    # Convert path for MSYS2 properly
    convert_path_for_msys2("${CMAKE_INSTALL_PREFIX}" INSTALL_PREFIX)
    set(VPX_TARGET --target=x86_64-win64-vs17)
    set(VPX_CXX_FLAGS "")
    set(VPX_C_FLAGS "")
    set(VPX_DEPENDS)
endif()

if(APPLE)
    set(VPX_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(VPX_C_FLAGS "${CMAKE_C_FLAGS}")
    if (CMAKE_OSX_DEPLOYMENT_TARGET)
        list(APPEND VPX_CXX_FLAGS "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
        list(APPEND VPX_C_FLAGS "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()
endif()

set(VPX_CONFIGURE_ARGS
    --prefix=${INSTALL_PREFIX}
    ${VPX_TARGET}
    --enable-pic
    --disable-examples
    --disable-tools
    --disable-docs
    --disable-unit-tests
    --enable-vp9-highbitdepth
    --extra-cflags=${VPX_C_FLAGS}
    --extra-cxxflags=${VPX_CXX_FLAGS}
    --as=nasm
)


set(VPX_INSTALL $make install)
if (WIN32)
    # Properly format VPX_CONFIGURE_ARGS
    list(JOIN VPX_CONFIGURE_ARGS " " VPX_CONFIGURE_ARGS_STR)

    set(VPX_CONFIGURE ${VPX_MSYS2} -c "pacman -S make nasm diffutils --noconfirm && ./configure ${VPX_CONFIGURE_ARGS_STR}")

    set(VPX_BUILD ${VPX_MSYS2} -c "make -j ${NPROCS}")

    set(VPX_INSTALL ${VPX_MSYS2} -c "make install && mv ${INSTALL_PREFIX}/lib/x64/vpxmd.lib ${INSTALL_PREFIX}/lib/vpx.lib" )

else()

    set(VPX_CONFIGURE cmake -E env ${VPX_ENV} --
	./configure ${VPX_CONFIGURE_ARGS})

    set(VPX_BUILD cmake -E env ${VPX_ENV} --
	make -j ${NPROCS})

    set(VPX_INSTALL cmake -E env ${VPX_ENV} --
	make install)
endif()

ExternalProject_Add(
    VPX
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/VPX
    DEPENDS ${VPX_DEPENDS}
    GIT_REPOSITORY "https://github.com/webmproject/libvpx.git"
    GIT_TAG ${VPX_TAG}
    
    CONFIGURE_COMMAND ${VPX_CONFIGURE}
    BUILD_COMMAND ${VPX_BUILD}
    INSTALL_COMMAND ${VPX_INSTALL}
    BUILD_IN_SOURCE 1
)

