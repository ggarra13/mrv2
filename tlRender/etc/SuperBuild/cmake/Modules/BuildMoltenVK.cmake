include(ExternalProject)

set(MoltenVK_GIT_REPOSITORY "https://github.com/KhronosGroup/MoltenVK")
set(MoltenVK_GIT_TAG ec7ccbc33006c13421f1e64f51a4f918abec2c48)


ExternalProject_Add(
    MoltenVK
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/MoltenVK
    GIT_REPOSITORY ${MoltenVK_GIT_REPOSITORY}
    GIT_TAG ${MoltenVK_GIT_TAG}
    CONFIGURE_COMMAND ./fetchDependencies --macos
    BUILD_COMMAND make macos
    INSTALL_COMMAND cp Package/Latest/MoltenVK/dynamic/dylib/macOS/libMoltenVK.dylib ${CMAKE_INSTALL_PREFIX}/lib
    )
