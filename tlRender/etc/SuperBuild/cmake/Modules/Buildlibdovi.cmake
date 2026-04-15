include(ExternalProject)


set(libdovi_GIT_REPO "https://github.com/quietvoid/dovi_tool.git")
set(libdovi_GIT_TAG libdovi-3.3.2)

set(libdovi_DEPENDENCIES )
message(STATUS "dovi DEPENDENCIES=${libdovi_DEPENDENCIES}")

set(INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})    
if (WIN32)
    include(functions/Msys2)
    convert_path_for_msys2("${CMAKE_INSTALL_PREFIX}" INSTALL_PREFIX)
endif()

set(libdovi_PATCH "" )
set(libdovi_CONFIGURE  "" )
set(libdovi_BUILD "")
if (WIN32)
    set(libdovi_move_lib mv ${INSTALL_PREFIX}/lib/dovi.dll.lib ${INSTALL_PREFIX}/lib/dovi.lib) 
    set(libdovi_INSTALL ${MRV2_MSYS_CMD} -c "([ -f /usr/bin/link.exe ] && mv -f /usr/bin/link /usr/bin/link_msys.exe) || true && cd dolby_vision && cargo cinstall --release --prefix ${INSTALL_PREFIX}" && ${libdovi_move_lib})
else()
    set(libdovi_INSTALL cd dolby_vision && cargo cinstall --release --prefix "${INSTALL_PREFIX}" --libdir=lib)
endif()


ExternalProject_Add(
    libdovi
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libdovi
    GIT_REPOSITORY ${libdovi_GIT_REPO}
    GIT_TAG ${libdovi_GIT_TAG}

    CONFIGURE_COMMAND ""
    PATCH_COMMAND ${libdovi_PATCH}
    BUILD_COMMAND ""
    INSTALL_COMMAND ${libdovi_INSTALL}
    BUILD_IN_SOURCE 1
)

