
include( ExternalProject )


ExternalProject_Add(
  GETTEXT
  URL "https://ftp.gnu.org/gnu/gettext/gettext-0.21.1.tar.gz"
  CONFIGURE_COMMAND ./configure --enable-relocatable
  --prefix=${CMAKE_INSTALL_PREFIX}
  "CFLAGS=${CMAKE_C_FLAGS}"
  "CPPFLAGS=${CMAKE_C_FLAGS}"
  "CXXFLAGS=${CMAKE_CXX_FLAGS}"
  BUILD_IN_SOURCE 1
  )

set( Gettext GETTEXT )
