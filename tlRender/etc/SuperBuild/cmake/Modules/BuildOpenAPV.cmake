if (USE_SYSTEM_LIBS)
    find_package(OAPV CONFIG)
    set(OpenAPV_DEP )
endif()

if (NOT OpenAPV_FOUND)
    include(ExternalProject)

    set(OpenAPV_GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/openapv.git")
    set(OpenAPV_GIT_TAG "v1.1.2.0") # v1.1.1.0 uses patch

    set(OpenAPV_DEPENDENCIES )
    set(OpenAPV_PATCH)

    if (WIN32)
	# To send upstream; drop it once a release carries the fix.
	#
	# An MSVC build compiles and runs, but installs no pkg-config file,
	# which is how FFmpeg finds the library, and gives the AVX sources no
	# /arch:AVX2 (openapv-patch/msvc.patch).
	find_package(Git REQUIRED)

	set(OpenAPV_PATCH
	    ${CMAKE_COMMAND}
            -DGIT_EXECUTABLE=${GIT_EXECUTABLE}
            -DPATCH_SOURCE_DIR=${CMAKE_CURRENT_BINARY_DIR}/../../../deps/OpenAPV/src/OpenAPV
            -DPATCH_FILE=${CMAKE_CURRENT_SOURCE_DIR}/patches/OpenAPV-patch/msvc.patch
            -P ${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake
	    )
    endif()

    set(OpenAPV_ARGS
	${TLRENDER_EXTERNAL_ARGS}
	-DOAPV_BUILD_APPS=OFF
	-DOAPV_BUILD_SHARED_LIB=OFF
	-DOAPV_BUILD_STATIC_LIB=ON
	-DCMAKE_INSTALL_LIBDIR=lib
	-DENABLE_TESTS=OFF
	-DCMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP=TRUE
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON)

    ExternalProject_Add(
	OpenAPV
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/../../../deps/OpenAPV
	GIT_REPOSITORY ${OpenAPV_GIT_REPOSITORY}
	GIT_TAG ${OpenAPV_GIT_TAG}
	PATCH_COMMAND ${OpenAPV_PATCH}
	
	DEPENDS ${OpenAPV_DEPENDENCIES}
	
	LIST_SEPARATOR |
	CMAKE_ARGS ${OpenAPV_ARGS})


    set(OpenAPV_DEP OpenAPV)
endif()
