# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

#-*-cmake-*-
#
# Test for LIBINTL (Color Transform Language)
#
# Once loaded this will define
#  LIBINTL_FOUND        - system has OpenEXR
#  LIBINTL_INCLUDE_DIR  - include directory for OpenEXR
#  LIBINTL_LIBRARIES    - libraries you need to link to
#


set(LIBINTL_FOUND "NO")

if( LIBINTL_LIBRARY_DIR )
  set( SEARCH_DIRS "${LIBINTL_LIBRARY_DIR}" )
else()
  set( SEARCH_DIRS
    "$ENV{LIBINTL_ROOT}/bin"
    "$ENV{LIBINTL_ROOT}/lib"
    "${CMAKE_PREFIX_PATH}/lib"
    /usr/local/lib
    /usr/lib
    )
endif()


find_path( LIBINTL_INCLUDE_DIR libintl.h
  DOC   "libintl includes"
)



 if( WIN32 OR APPLE )


   FIND_LIBRARY( libintl
     NAMES libintl intl
     NO_DEFAULT_PATH
     PATHS ${SEARCH_DIRS}
     DOC   "LIBINTL library"
   )
   
   set(LIBINTL_LIBRARIES ${libintl} )
else( WIN32 OR APPLE )
  set( LIBINTL_LIBRARIES "" )  # on linux, it resides on libc
endif( WIN32 OR APPLE )



if(NOT LIBINTL_FOUND)
  if (LIBINTL_INCLUDE_DIR)
    if(LIBINTL_LIBRARIES OR LIBINTL_LIBRARIES STREQUAL "")
      set(LIBINTL_FOUND "YES")
    endif(LIBINTL_LIBRARIES OR LIBINTL_LIBRARIES STREQUAL "")
  endif(LIBINTL_INCLUDE_DIR)
endif(NOT LIBINTL_FOUND)

if(NOT LIBINTL_FOUND)
  # make FIND_PACKAGE friendly
  if(NOT LIBINTL_FIND_QUIETLY)
    if(LIBINTL_FIND_REQUIRED)
      MESSAGE( STATUS "LIBINTL_INCLUDE_DIR ${LIBINTL_INCLUDE_DIR}" )
      MESSAGE( STATUS "LIBINTL_LIBRARIES   ${LIBINTL_LIBRARIES}" )
      MESSAGE(FATAL_ERROR
	      "LIBINTL required, please specify its location with LIBINTL_ROOT.")
    else()
      MESSAGE(FATAL_ERROR "LIBINTL was not found. ${LIBINTL_INCLUDE_DIR} ${LIBINTL_LIBRARIES}")
    endif()
  endif()
endif()

#####
