include(ExternalProject)


set(libdovi_GIT_REPO "https://github.com/quietvoid/dovi_tool.git")
set(libdovi_GIT_TAG libdovi-3.3.2)

set(libdovi_DEPENDENCIES )
message(STATUS "dovi DEPENDENCIES=${libdovi_DEPENDENCIES}")

if (WIN32)
    include(functions/Msys2)
endif()

set(INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
if(WIN32)
    convert_path_for_msys2("${CMAKE_INSTALL_PREFIX}" INSTALL_PREFIX)
endif()

set(libdovi_PATCH )
if (WIN32)
    set(libdovi_CONFIGURE )
    set(libdovi_INSTALL ${MRV2_MSYS_CMD} -c "cd dolby_vision && cargo cinstall --release --prefix ${INSTALL_PREFIX}")
else()
    set(libdovi_CONFIGURE )
    set(libdovi_INSTALL cd dolby_vision && cargo cinstall --release --prefix "${INSTALL_PREFIX}")
endif()


ExternalProject_Add(
    libdovi
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libdovi
    GIT_REPOSITORY ${libdovi_GIT_REPO}
    GIT_TAG ${libdovi_GIT_TAG}
    DEPENDS ${libdovi_DEPENDENCIES}
    CONFIGURE_COMMAND ${libdovi_CONFIGURE}
    PATCH_COMMAND ${libdovi_PATCH}
    BUILD_COMMAND ${libdovi_BUILD}
    INSTALL_COMMAND ${libdovi_INSTALL}
    BUILD_IN_SOURCE 1
)

