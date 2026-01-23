
set(HARFBUZZ_DEP )

# Linux harfbuzz is already installed as a depedency of FLTK.
if (APPLE OR WIN32)

    include(ExternalProject)

    set(HARFBUZZ_GIT_REPOSITORY "https://github.com/harfbuzz/harfbuzz.git")
    set(HARFBUZZ_GIT_TAG "12.3.1")

    set(HARFBUZZ_DEPENDENCIES Freetype)

    set(HARFBUZZ_ARGS
	${TLRENDER_EXTERNAL_ARGS}
	-DHB_HAVE_FREETYPE=ON)

    ExternalProject_Add(
	harfbuzz
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/harfbuzz
	GIT_REPOSITORY ${HARFBUZZ_GIT_REPOSITORY}
	GIT_TAG ${HARFBUZZ_GIT_TAG}
	DEPENDS ${HARFBUZZ_DEPENDENCIES}
	LIST_SEPARATOR |
	CMAKE_ARGS ${HARFBUZZ_ARGS})

    set(HARFBUZZ_DEP harfbuzz)
endif()
