cmake_minimum_required(VERSION 3.15)
project(lcms2 LANGUAGES C VERSION 2.16) # adjust version

# Sources
file(GLOB SRC src/*.c)
add_library(lcms2 ${SRC})
target_include_directories(lcms2
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

set_target_properties(lcms2 PROPERTIES
    OUTPUT_NAME lcms2
    EXPORT_NAME lcms2
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Install rules
include(GNUInstallDirs)

install(TARGETS lcms2
    EXPORT LCMS2Targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export targets for downstream usage
install(EXPORT LCMS2Targets
    FILE LCMS2Targets.cmake
    NAMESPACE LCMS::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LCMS2
)

# Generate a Config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/LCMS2Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LCMS2Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LCMS2
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/LCMS2ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/LCMS2Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/LCMS2ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LCMS2
)
