# lunasvg updates often, so we alwaycs build it.

# if (USE_SYSTEM_LIBS)
#     find_package(lunasvg CONFIG)
#     set(lunasvg_DEP )
# endif()

if (NOT lunasvg_FOUND)
    include(ExternalProject)

    set(lunasvg_GIT_REPOSITORY "https://github.com/sammycage/lunasvg.git")
    set(lunasvg_GIT_TAG "v3.5.0")

    set(lunasvg_DEPENDENCIES )
    message(STATUS "lunasvg DEPENDENCIES=${lunasvg_DEPENDENCIES}")

    set(lunasvg_PATCH )
    
    set(lunasvg_ARGS
	${TLRENDER_EXTERNAL_ARGS}
	-DLUNASVG_BUILD_EXAMPLES=OFF
	-DPLUTOVG_BUILD_EXAMPLES=OFF)

    ExternalProject_Add(
	lunasvg
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/../../../deps/lunasvg
	DEPENDS ${lunasvg_DEPENDENCIES}
	GIT_REPOSITORY ${lunasvg_GIT_REPOSITORY}
	GIT_TAG ${lunasvg_GIT_TAG}

	PATCH_COMMAND ${lunasvg_PATCH}
	
	LIST_SEPARATOR |
	CMAKE_ARGS ${lunasvg_ARGS})

    set(lunasvg_DEP lunasvg)

endif()
