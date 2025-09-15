include(ExternalProject)

set(OpenJPH_GIT_REPOSITORY "https://github.com/aous72/OpenJPH.git")
set(OpenJPH_GIT_TAG "0.23.0")

set( OpenJPH_C_COMPILER $ENV{NATIVE_C_COMPILER})
set( OpenJPH_CXX_COMPILER $ENV{NATIVE_CXX_COMPILER})


if (WIN32)
    # \@bug: Currently, OpenJPH does not build with the Microsoft ARM64
    #        compiler
    if ($ENV{ARCH} MATCHES ".*aarch64.*" OR $ENV{ARCH} MATCHES ".*arm64.*")
	set( OpenJPH_C_COMPILER $ENV{GENERIC_C_COMPILER})
	set( OpenJPH_CXX_COMPILER $ENV{GENERIC_CXX_COMPILER})
    endif()
endif()

set(OpenJPH_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DCMAKE_C_COMPILER=${OpenJPH_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${OpenJPH_CXX_COMPILER})

ExternalProject_Add(
    OpenJPH
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenJPH
    GIT_REPOSITORY ${OpenJPH_GIT_REPOSITORY}
    GIT_TAG ${OpenJPH_GIT_TAG}
    
    LIST_SEPARATOR |
    CMAKE_ARGS ${OpenJPH_ARGS})

set(OpenJPH_DEP OpenJPH)
