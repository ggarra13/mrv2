include( ExternalProject )

set( FLTK_TAG master )

set( wayland OFF )#  we'll leave it off as it is still too buggy.
set( patch_cmd )

if (APPLE)
#  set( patch_cmd ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/patches/FLTK/Fl_Cocoa_Window_Driver.cxx ${CMAKE_BINARY_DIR}/FLTK-prefix/src/FLTK/src/drivers/Cocoa )
  set( wayland OFF )  
endif()

ExternalProject_Add(
  FLTK
  GIT_REPOSITORY "https://github.com/fltk/fltk.git"
  GIT_TAG ${FLTK_TAG}
  GIT_PROGRESS 1
  PATCH_COMMAND ${patch_cmd}
  CMAKE_ARGS
  -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DFLTK_BUILD_EXAMPLES=OFF
  -DFLTK_BUILD_TEST=OFF
  -DOPTION_BUILD_SHARED_LIBS=0
  -DOPTION_USE_SYSTEM_ZLIB=0
  -DOPTION_USE_SYSTEM_LIBJPEG=0
  -DOPTION_USE_SYSTEM_LIBPNG=0
  -DOPTION_USE_WAYLAND=${wayland}
)
