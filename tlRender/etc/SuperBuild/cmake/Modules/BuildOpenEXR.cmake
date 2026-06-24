include(ExternalProject)

if (USE_SYSTEM_LIBS)
    find_package(OpenEXR)
    set(OpenEXR_DEP )
endif()

if (NOT OpenEXR_FOUND)
    set(OpenEXR_GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/openexr.git")
    set(OpenEXR_GIT_TAG "v3.4.12")  # was v3.4.9

    set(OpenEXR_DEPENDENCIES ${OpenJPH_DEP} Imath ZLIB)
    message(STATUS "OpenEXR DEPENDENCIES=${OpenEXR_DEPENDENCIES}")

    # \bug Disable OpenEXR threading to work around a crash at shutdown in the
    #      OpenEXR thread pool. Note that we already set the OpenEXR global thread
    #      count to zero wiht Imf::setGlobalThreadCount(0), since we load frames in
    #      parallel.
    set(OpenEXR_ARGS
	${TLRENDER_EXTERNAL_ARGS}
	-DOPENEXR_BUILD_TOOLS=OFF
	-DOPENEXR_BUILD_EXAMPLES=OFF
	
	-DOPENEXR_INSTALL_EXAMPLES=OFF
	-DOPENEXR_INSTALL_TOOLS=OFF
	
	-DBUILD_TESTING=OFF
	-DOPENEXR_TEST_LIBRARIES=OFF
	-DOPENEXR_TEST_PYTHON=OFF
	-DOPENEXR_TEST_TOOLS=OFF

	-DOPENEXR_ENABLE_THREADING=ON
	-DOPENEXR_FORCE_INTERNAL_OPENJPH=OFF
	-DOPENEXR_FORCE_INTERNAL_DEFLATE=ON)

    ExternalProject_Add(
	OpenEXR
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenEXR
	DEPENDS ${OpenEXR_DEPENDENCIES}
	GIT_REPOSITORY ${OpenEXR_GIT_REPOSITORY}
	GIT_TAG ${OpenEXR_GIT_TAG}

	PATCH_COMMAND ${OpenEXR_PATCH}
	
	LIST_SEPARATOR |
	CMAKE_ARGS ${OpenEXR_ARGS})

    set(OpenEXR_DEP OpenEXR)

endif()
