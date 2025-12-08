include(ExternalProject)

include(ProcessorCount)
ProcessorCount(NPROCS)

set(OpenSSL_GIT_REPOSITORY "https://github.com/openssl/openssl.git")
set(OpenSSL_GIT_TAG "openssl-3.3.0")

set(OpenSSL_DEPENDENCIES )
if(NOT WIN32)
    list(APPEND OpenSSL_DEPENDENCIES NASM)
endif()
list(APPEND OpenSSL_DEPENDENCIES ZLIB)
message(STATUS "OpenSSL DEPENDENCIES=${OpenSSL_DEPENDENCIES}")

#
# Set up the dependency
#
set(OpenSSL_DEP )
set(OpenSSL_DEP PARENT_SCOPE )

if(WIN32)
    # Define the default, and most common, Strawberry Perl path
    set(STRAWBERRY_PERL_ROOT "C:/Strawberry")

    # Use CMake's find_program with the HINTS argument.
    # HINTS gives a set of paths to check *before* looking in the system's PATH.
    find_program(PERL_EXE
        NAMES perl.exe
        HINTS "${STRAWBERRY_PERL_ROOT}/perl/bin"
        DOC "Path to the Strawberry Perl executable"
    )

    # If PERL_EXE is not found, it's still possible that the user has a custom
    # installation. In that case, we can provide a helpful error message.
    if(NOT PERL_EXE)
	message(FATAL_ERROR "Could not find Strawberry Perl. Please ensure that the 'PERL_EXE' CMake variable STRAWBERRY_PERL_ROOT is set correctly, or that Strawberry Perl is installed to the default location.")
    endif()
    
    if ($ENV{ARCH} MATCHES ".*aarch64.*" OR $ENV{ARCH} MATCHES ".*arm64.*")
	set(OpenSSL_CONFIGURE
	    ${PERL_EXE} Configure VC-WIN64-ARM
	    --prefix=${CMAKE_INSTALL_PREFIX}
	    --openssldir=${CMAKE_INSTALL_PREFIX}
	    no-external-tests
	    no-tests
	    no-unit-test)
	set(OpenSSL_BUILD nmake install)
	set(OpenSSL_INSTALL nmake install)
    elseif ($ENV{ARCH} MATCHES ".*amd64.*")
	set(OpenSSL_CONFIGURE
	    ${PERL_EXE} Configure VC-WIN64A
	    --prefix=${CMAKE_INSTALL_PREFIX}
	    --openssldir=${CMAKE_INSTALL_PREFIX}
	    no-external-tests
	    no-tests
	    no-unit-test)
	set(OpenSSL_BUILD nmake install)
	set(OpenSSL_INSTALL nmake install)
    else()
	message(FATAL_ERROR "Windows architecture is incompatible")
    endif()
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


#
# Download / Install
#

if(NOT WIN32)
    ExternalProject_Add(
	OpenSSL
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenSSL
	DEPENDS ${OpenSSL_DEPENDENCIES}
	GIT_REPOSITORY ${OpenSSL_GIT_REPOSITORY}
	GIT_TAG ${OpenSSL_GIT_TAG}
	CONFIGURE_COMMAND ${OpenSSL_CONFIGURE}
	BUILD_COMMAND ${OpenSSL_BUILD}
	INSTALL_COMMAND ${OpenSSL_INSTALL}
	BUILD_IN_SOURCE 1)

    set(OpenSSL_DEP OpenSSL)
    set(OpenSSL_DEP OpenSSL PARENT_SCOPE)
else()
    if (USE_VCPKG)
	add_custom_target(
	    OpenSSL ALL
	    ${CMAKE_COMMAND} -E echo "Installing openssl..."
	    COMMAND ${VCPKG} install openssl:${VCPKG_TRIPLET}
	    DEPENDS ${vcpkg_DEP} ${Gettext_DEP}) # Gettext_DEP needed

	# \bug Copy libssl.lib to ssl.lib and libcrypto.lib to crypto.lib so the
	# FFmpeg configure script can find them.
	# Not done here, but in pre-flight script of mrv2
	add_custom_target(
	    OpenSSL_install ALL
	    ${CMAKE_COMMAND} -E echo "Copying openssl directories to CMAKE_INSTALL_PREFIX..."
	    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${VCPKG_LIB_DIR} ${CMAKE_INSTALL_PREFIX}/lib
	    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${VCPKG_INCLUDE_DIR} ${CMAKE_INSTALL_PREFIX}/include
	    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${VCPKG_BIN_DIR} ${CMAKE_INSTALL_PREFIX}/bin
	    DEPENDS OpenSSL
	)
	set(OpenSSL_DEP OpenSSL_install)
    else()
	ExternalProject_Add(
	    OpenSSL
	    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenSSL
	    DEPENDS ${OpenSSL_DEPENDENCIES}
	    GIT_REPOSITORY ${OpenSSL_GIT_REPOSITORY}
	    GIT_TAG ${OpenSSL_GIT_TAG}
	    CONFIGURE_COMMAND ${OpenSSL_CONFIGURE}
	    BUILD_COMMAND ${OpenSSL_BUILD}
	    INSTALL_COMMAND ${OpenSSL_INSTALL}
	    BUILD_IN_SOURCE 1)
	
	set(OpenSSL_DEP OpenSSL)
    endif()
    
    # \bug Copy libssl.lib to ssl.lib and libcrypto.lib to crypto.lib so the
    # FFmpeg configure script can find them.
    add_custom_target(
	OpenSSL_rename ALL
	${CMAKE_COMMAND} -E echo "Copying openssl libraries for FFmpeg..."
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/libssl.lib ${CMAKE_INSTALL_PREFIX}/lib/ssl.lib 
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/lib/libcrypto.lib ${CMAKE_INSTALL_PREFIX}/lib/crypto.lib 
	DEPENDS ${OpenSSL_DEP}
    )
    
    set(OpenSSL_DEP OpenSSL_rename)
    set(OpenSSL_DEP OpenSSL_rename PARENT_SCOPE)
endif()

