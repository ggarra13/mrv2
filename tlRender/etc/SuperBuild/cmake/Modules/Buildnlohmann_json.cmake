include(ExternalProject)

if(TLRENDER_LOCAL)
    set(nlohmann_json_GIT_REPOSITORY "git@github.com:ggarra13/json.git")
else()
    set(nlohmann_json_GIT_REPOSITORY "https://github.com/nlohmann/json.git") # orig
endif()
set(nlohmann_json_GIT_TAG "v3.11.3")

set(nlohmann_json_ARGS ${TLRENDER_EXTERNAL_ARGS})
list(APPEND nlohmann_json_ARGS -DJSON_BuildTests=OFF)

ExternalProject_Add(
    nlohmann_json
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/nlohmann_json
    GIT_REPOSITORY ${nlohmann_json_GIT_REPOSITORY}
    GIT_TAG ${nlohmann_json_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${nlohmann_json_ARGS}
)
