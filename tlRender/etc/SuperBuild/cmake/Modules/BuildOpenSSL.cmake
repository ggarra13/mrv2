include(ExternalProject)

set(OpenSSL_DEPENDS)
if(NOT WIN32)
    list(APPEND OpenSSL_DEPENDS NASM)
endif()
list(APPEND OpenSSL_DEPENDS ZLIB)

set(OpenSSL_GIT_REPOSITORY "https://github.com/openssl/openssl.git")
set(OpenSSL_GIT_TAG "openssl-3.3.0")

set(OpenSSL )

include(ProcessorCount)
ProcessorCount(NPROCS)

if(WIN32)
    #
    # We build with MSys
    #
    # set(OpenSSL_CONFIGURE
    #     perl Configure VC-WIN64A
    #     --prefix=${CMAKE_INSTALL_PREFIX}
    #     --openssldir=${CMAKE_INSTALL_PREFIX}
    #     no-external-tests
    #     no-tests
    #     no-unit-test)
    # set(OpenSSL_BUILD nmake install)
    # set(OpenSSL_INSTALL nmake install)
elseif(APPLE)
    set(OpenSSL_CONFIGURE
        ./Configure
        --prefix=${CMAKE_INSTALL_PREFIX}
        --openssldir=${CMAKE_INSTALL_PREFIX}
        no-docs
        no-external-tests
	no-shared
        no-tests
        no-unit-test)
    if(CMAKE_OSX_DEPLOYMENT_TARGET)
        list(APPEND OpenSSL_CONFIGURE -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    endif()
    set(OpenSSL_BUILD make -j ${NPROCS})
    set(OpenSSL_INSTALL make -j ${NPROCS} install &&
	rm -f ${CMAKE_INSTALL_PREFIX}/bin/openssl)
else()
    set(OpenSSL_CONFIGURE
        ./Configure
        --prefix=${CMAKE_INSTALL_PREFIX}
        --openssldir=${CMAKE_INSTALL_PREFIX}
        no-docs
        no-external-tests
	no-shared
        no-tests
        no-unit-test)
    set(OpenSSL_BUILD make -j ${NPROCS})
    set(OpenSSL_INSTALL make -j ${NPROCS} install &&
	rm -f ${CMAKE_INSTALL_PREFIX}/bin/openssl)

endif()

if(NOT WIN32)
    ExternalProject_Add(
	OpenSSL
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenSSL
	DEPENDS ${OpenSSL_DEPENDS}
	GIT_REPOSITORY ${OpenSSL_GIT_REPOSITORY}
	GIT_TAG ${OpenSSL_GIT_TAG}
	CONFIGURE_COMMAND ${OpenSSL_CONFIGURE}
	BUILD_COMMAND ${OpenSSL_BUILD}
	INSTALL_COMMAND ${OpenSSL_INSTALL}
	BUILD_IN_SOURCE 1)

    set(OpenSSL OpenSSL)
endif()
