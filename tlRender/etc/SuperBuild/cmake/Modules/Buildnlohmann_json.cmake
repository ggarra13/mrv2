if (USE_SYSTEM_LIBS)
    find_package(nlohmann-json)
    set(nlohmann-json_DEP )
endif()

if (NOT nlohmann-json_FOUND)
    include(ExternalProject)

    set(nlohmann-json_GIT_REPOSITORY "https://github.com/nlohmann/json.git")
    set(nlohmann-json_GIT_TAG "v3.11.3")

    set(nlohmann-json_DEPENDENCIES )

    set(nlohmann-json_ARGS ${TLRENDER_EXTERNAL_ARGS})
    list(APPEND nlohmann-json_ARGS -DJSON_BuildTests=OFF)

    ExternalProject_Add(
	nlohmann-json
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/nlohmann-json
	GIT_REPOSITORY ${nlohmann-json_GIT_REPOSITORY}
	GIT_TAG ${nlohmann-json_GIT_TAG}

	DEPENDS ${nlohmann-json_DEPENDENCIES}

	LIST_SEPARATOR |
	CMAKE_ARGS ${nlohmann-json_ARGS}
    )

    set(nlohmann-json_DEP nlohmann-json)

endif()
