
set(HARFBUZZ_DEP )

# Linux harfbuzz is already installed as a depedency of FLTK.
if (APPLE OR WIN32)

    include(ExternalProject)

    set(harfbuzz_GIT_REPOSITORY "https://github.com/harfbuzz/harfbuzz.git")
    set(harfbuzz_GIT_TAG "12.3.1")

    set(harfbuzz_DEPENDENCIES ${PYTHON_DEP})
    list(APPEND harfbuzz_DEPENDENCIES Freetype)

    message(STATUS "harfbuzz DEPENDENCIES=${harfbuzz_DEPENDENCIES}")

    set(harfbuzz_PATCH )

    set(harfbuzz_ARGS
	${TLRENDER_EXTERNAL_ARGS}
	-DHB_HAVE_FREETYPE=ON)

    ExternalProject_Add(
	harfbuzz
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/harfbuzz
	GIT_REPOSITORY ${harfbuzz_GIT_REPOSITORY}
	GIT_TAG ${harfbuzz_GIT_TAG}
	DEPENDS ${harfbuzz_DEPENDENCIES}
	PATCH_COMMAND  ${harfbuzz_PATCH}
	LIST_SEPARATOR |
	CMAKE_ARGS ${harfbuzz_ARGS})

    set(harfbuzz_DEP harfbuzz)
endif()
