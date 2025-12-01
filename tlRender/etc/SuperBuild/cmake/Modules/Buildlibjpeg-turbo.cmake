include(ExternalProject)

set(libjpeg-turbo_GIT_REPOSITORY "https://github.com/libjpeg-turbo/libjpeg-turbo.git")
set(libjpeg-turbo_GIT_TAG "3.1.0") # was 3.0.0

set(libjpeg-turbo_DEPENDENCIES ZLIB)
if(NOT WIN32)
    if (NOT "$ENV{ARCH}" MATCHES ".*aarch64.*")
       set(libjpeg-turbo_DEPENDENCIES ${libjpeg-turbo_DEPENDENCIES}  NASM)
    endif()
endif()		
message(STATUS "libjpeg-turbo DEPENDENCIES=${libjpeg-turbo_DEPENDENCIES}")

set(libjpeg-turbo_ENABLE_SHARED ON)
set(libjpeg-turbo_ENABLE_STATIC OFF)
if(NOT BUILD_SHARED_LIBS)
    set(libjpeg-turbo_ENABLE_SHARED OFF)
    set(libjpeg-turbo_ENABLE_STATIC ON)
endif()

set(libjpeg-turbo_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_PREFIX}/lib
    -DENABLE_SHARED=${libjpeg-turbo_ENABLE_SHARED}
    -DENABLE_STATIC=${libjpeg-turbo_ENABLE_STATIC}
    -DWITH_TESTS=OFF
    -DWITH_TOOLS=OFF)
if(NOT WIN32)
    if (NOT "$ENV{ARCH}" MATCHES ".*aarch64.*")
	list(APPEND libjpeg-turbo_ARGS -DCMAKE_ASM_NASM_COMPILER=${CMAKE_INSTALL_PREFIX}/bin/nasm)
     else()	    
	list(APPEND libjpeg-turbo_ARGS -DWITH_SIMD=OFF)
    endif()
    if(NOT APPLE)
	list(APPEND libjpeg-turbo_ARGS -DCMAKE_POSITION_INDEPENDENT_CODE=ON)
    endif()
endif()

message(STATUS "libjpeg-turbo ARGS=${libjpeg-turbo_ARGS}")

ExternalProject_Add(
    libjpeg-turbo
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libjpeg-turbo
    DEPENDS ${libjpeg-turbo_DEPENDENCIES}
    GIT_REPOSITORY ${libjpeg-turbo_GIT_REPOSITORY}
    GIT_TAG ${libjpeg-turbo_GIT_TAG}
    
    LIST_SEPARATOR |
    CMAKE_ARGS ${libjpeg-turbo_ARGS})
