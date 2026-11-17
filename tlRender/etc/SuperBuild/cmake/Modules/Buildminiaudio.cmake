include(ExternalProject)

set(miniaudio_GIT_REPOSITORY "https://github.com/mackron/miniaudio.git")
set(miniaudio_GIT_TAG "0.11.26") # latest Oct 31, 2026
set(miniaudio_PATCH )

set(miniaudio_DEPENDENCIES )
message(STATUS "miniaudio DEPENDENCIES=${miniaudio_DEPENDENCIES}")

set(miniaudio_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DCMAKE_DEBUG_POSTFIX=
    -DMINIAUDIO_BUILD_TESTS=FALSE
    -DMINIAUDIO_BUILD_TOOLS=FALSE)
if (UNIX AND NOT APPLE)
    list(APPEND miniaudio_ARGS
	-DMINIAUDIO_ENABLE_ALSA=ON
	-DMINIAUDIO_ENABLE_JACK=OFF
	-DMINIAUDIO_ENABLE_PULSEAUDIO=ON)
elseif(APPLE)
    list(APPEND miniaudio_ARGS
	-DMINIAUDIO_ENABLE_COREAUDIO=ON
	-DMINIAUDIO_NO_RUNTIME_LINKING=ON)
elseif(WIN32)
    list(APPEND miniaudio_ARGS
	-DMINIAUDIO_ENABLE_WASAPI=ON)
endif()

ExternalProject_Add(
    miniaudio
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/../../../deps/miniaudio
    GIT_REPOSITORY ${miniaudio_GIT_REPOSITORY}
    GIT_TAG ${miniaudio_GIT_TAG}
    
    DEPENDS ${miniaudio_DEPENDENCIES}

    PATCH_COMMAND ${miniaudio_PATCH}

    LIST_SEPARATOR |

    CMAKE_ARGS ${miniaudio_ARGS})

set(miniaudio_DEP miniaudio)
