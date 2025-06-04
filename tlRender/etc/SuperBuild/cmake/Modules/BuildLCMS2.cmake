
include(ProcessorCount)
ProcessorCount(NPROCS)

if(TLRENDER_LOCAL)
    set(LCMS2_GIT_REPO "git@github.com:ggarra13/Little-CMS.git")
else()
    set(LCMS2_GIT_REPO "https://github.com/mm2/Little-CMS.git")
endif()

set(LCMS2_GIT_TAG lcms2.17)

if(APPLE)
    set(LCMS2_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(LCMS2_C_FLAGS "${CMAKE_C_FLAGS}")
    if (CMAKE_OSX_DEPLOYMENT_TARGET)
        list(APPEND LCMS2_CXX_FLAGS "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
        list(APPEND LCMS2_C_FLAGS "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()
endif()

if (WIN32)
    include(functions/Msys2)
    # Convert path for MSYS2 properly
    convert_path_for_msys2("${CMAKE_INSTALL_PREFIX}" INSTALL_PREFIX)
else()
    set(INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
endif()
    
set(LCMS2_CONFIGURE_ARGS
    --enable-shared
    --disable-static
    --prefix=${INSTALL_PREFIX}
    "CFLAGS=${LCMS2_C_FLAGS}"
    "CXXFLAGS=${LCMS2_CXX_FLAGS}"
)
    
set(LCMS2_CONFIGURE_COMMAND ./configure ${LCMS2_CONFIGURE_ARGS})
set(LCMS2_BUILD_COMMAND make -j ${NPROCS})
set(LCMS2_INSTALL_COMMAND make install)

if(WIN32)
    # Build LCMS2 with MSYS2 on Windows.
    find_package(Msys REQUIRED)
    set(LCMS2_MSYS2
        ${MSYS_CMD}
        -use-full-path
        -defterm
        -no-start
        -here)

    # Properly format VPX_CONFIGURE_ARGS
    list(JOIN LCMS2_CONFIGURE_ARGS " " LCMS2_CONFIGURE_ARGS_STR)

    set(LCMS2_CONFIGURE_COMMAND ${LCMS2_MSYS2} -c "CC=cl CXX=cl LD=link ./configure --build=mingw64 ${LCMS2_CONFIGURE_ARGS_STR}")
    set(LCMS2_BUILD_COMMAND ${LCMS2_MSYS2} -c "make -j ${NPROCS}")
    set(LCMS2_INSTALL_COMMAND ${LCMS2_MSYS2} -c "make install && mv ${INSTALL_PREFIX}/lib/lcms2.dll.lib ${INSTALL_PREFIX}/lib/liblcms2.lib")
endif()

ExternalProject_Add(
    LCMS2
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/LCMS2
    GIT_REPOSITORY ${LCMS2_GIT_REPO}
    GIT_TAG ${LCMS2_GIT_TAG}
    
    CONFIGURE_COMMAND ${LCMS2_CONFIGURE_COMMAND}
    BUILD_COMMAND ${LCMS2_BUILD_COMMAND}
    INSTALL_COMMAND ${LCMS2_INSTALL_COMMAND}
    BUILD_IN_SOURCE 1)
    
