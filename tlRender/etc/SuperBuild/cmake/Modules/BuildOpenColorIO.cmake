include(ExternalProject)

if (USE_SYSTEM_LIBS)
    find_package(OpenColorIO CONFIG)
    set(OpenColorIO_DEP )
endif()

if (NOT OpenColorIO_FOUND)

    set(OpenColorIO_GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/OpenColorIO.git")
    set(OpenColorIO_GIT_TAG "v2.5.2") # was v2.5.1

    set(OpenColorIO_DEPENDENCIES ${Imath_DEP} ${yaml-cpp_DEP} ${expat_DEP} ${pystring_DEP} ${minizip-ng_DEP} ${pystring_DEP} ${ZLIB_DEP})
    message(STATUS "OpenColorIO DEPENDENCIES=${OpenColorIO_DEPENDENCIES}")

    set(OpenColorIO_ARGS
	${TLRENDER_EXTERNAL_ARGS}
	-DCMAKE_INSTALL_LIBDIR=lib
	-DBUILD_TESTING=OFF
	-DOCIO_BUILD_APPS=OFF
	-DOCIO_BUILD_TESTS=OFF
	-DOCIO_BUILD_GPU_TESTS=OFF
	-DOCIO_BUILD_PYTHON=OFF
	-DOCIO_INSTALL_EXT_PACKAGES=NONE)

    ExternalProject_Add(
	OpenColorIO
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenColorIO
	DEPENDS ${OpenColorIO_DEPENDENCIES}
	GIT_REPOSITORY ${OpenColorIO_GIT_REPOSITORY}
	GIT_TAG ${OpenColorIO_GIT_TAG}
	
	LIST_SEPARATOR |
	CMAKE_ARGS ${OpenColorIO_ARGS})

    set(OpenColorIO_DEP OpenColorIO)

endif()
