include(ExternalProject)

set(MoltenVK_GIT_REPOSITORY "https://github.com/KhronosGroup/MoltenVK")
# set(MoltenVK_GIT_TAG ec7ccbc33006c13421f1e64f51a4f918abec2c48) # vmrv2 v1.4.3 - May 30
#set(MoltenVK_GIT_TAG v1.4.0)  # August 20
set(MoltenVK_GIT_TAG c2e5c5ecea0f5bdafad2d9a6eb21f0c33bb2e995) # Oct

ExternalProject_Add(
    MoltenVK
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/MoltenVK
    GIT_REPOSITORY ${MoltenVK_GIT_REPOSITORY}
    GIT_TAG ${MoltenVK_GIT_TAG}
    CONFIGURE_COMMAND ./fetchDependencies --macos
    BUILD_COMMAND make macos
    INSTALL_COMMAND cp Package/Latest/MoltenVK/dynamic/dylib/macOS/libMoltenVK.dylib ${CMAKE_INSTALL_PREFIX}/lib
    BUILD_IN_SOURCE 1
    )
