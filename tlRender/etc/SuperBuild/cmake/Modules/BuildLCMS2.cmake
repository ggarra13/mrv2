
set(LCMS2_GIT_REPO "https://github.com/mm2/Little-CMS.git")
set(LCMS2_GIT_TAG lcms2.17)

set(LCMS2_DEPENDENCIES )
message(STATUS "LCMS2 DEPENDENCIES=${LCMS2_DEPENDENCIES}")

set(LCMS2_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
)

#
# This copies the .cmake files from Libraw-cmake.  Two files need patching thou.
#
set(LCMS2_PATCH
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/patches/LCMS2-patch/CMakeLists.txt <SOURCE_DIR>
    COMMAND ${CMAKE_COMMAND} -E make_directory <SOURCE_DIR>/cmake
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/patches/LCMS2-patch/cmake/LCMS2Config.cmake.in <SOURCE_DIR>/cmake/LCMS2Config.cmake.in
)

ExternalProject_Add(
    LCMS2
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/LCMS2
    GIT_REPOSITORY ${LCMS2_GIT_REPO}
    GIT_TAG ${LCMS2_GIT_TAG}

    DEPENDS ${LCMS2_DEPENDENCIES}
    
    PATCH_COMMAND ${LCMS2_PATCH}
    
    LIST_SEPARATOR |
    CMAKE_ARGS ${LCMS2_ARGS})
    
