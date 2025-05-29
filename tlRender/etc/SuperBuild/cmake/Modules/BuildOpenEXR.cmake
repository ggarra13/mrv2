include(ExternalProject)

set(OpenEXR_GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/openexr.git")
set(OpenEXR_GIT_TAG "v3.3.2")

# \bug Disable OpenEXR threading to work around a crash at shutdown in the
# OpenEXR thread pool. Note that we already set the OpenEXR global thread
# count to zero since we load frames in parallel.
# \bug Disabling OpenEXR threading makes the library not thread safe currently.
set(OpenEXR_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DOPENEXR_BUILD_TOOLS=OFF
    -DOPENEXR_INSTALL_EXAMPLES=OFF
    -DBUILD_TESTING=OFF
    -DOPENEXR_ENABLE_THREADING=ON
    -DOPENEXR_FORCE_INTERNAL_DEFLATE=ON)

ExternalProject_Add(
    OpenEXR
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenEXR
    DEPENDS Imath ZLIB
    GIT_REPOSITORY ${OpenEXR_GIT_REPOSITORY}
    GIT_TAG ${OpenEXR_GIT_TAG}
    
    LIST_SEPARATOR |
    CMAKE_ARGS ${OpenEXR_ARGS})
