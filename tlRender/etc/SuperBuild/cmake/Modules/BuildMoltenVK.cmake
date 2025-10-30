include(ExternalProject)

set(MoltenVK_GIT_REPOSITORY "https://github.com/KhronosGroup/MoltenVK")
#set(MoltenVK_GIT_TAG v1.4.0)  # August 20
set(MoltenVK_GIT_TAG c2e5c5ecea0f5bdafad2d9a6eb21f0c33bb2e995) # Oct. 22

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
