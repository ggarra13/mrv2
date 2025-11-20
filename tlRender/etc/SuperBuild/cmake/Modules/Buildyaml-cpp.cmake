include(ExternalProject)

set(yaml-cpp_GIT_REPOSITORY "https://github.com/jbeder/yaml-cpp.git")
set(yaml-cpp_GIT_TAG "0.8.0")  # was yaml-cpp-0.7.0, not 0.7.0

set(yaml-cpp_DEPENDENCIES )

set(yaml-cpp_PATCH )

list(APPEND yaml-cpp_PATCH COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/patches/yaml-cpp-patch/src/emitterutils.cpp
	${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp/src/yaml-cpp/src/emitterutils.cpp)
       
set(yaml-cpp_ARGS
    -DYAML_CPP_BUILD_CONTRIB=OFF
    -DYAML_CPP_BUILD_TOOLS=OFF
    -DYAML_CPP_BUILD_TESTS=OFF
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    yaml-cpp
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp
    GIT_REPOSITORY ${yaml-cpp_GIT_REPOSITORY}
    GIT_TAG ${yaml-cpp_GIT_TAG}
    PATCH_COMMAND ${yaml-cpp_PATCH}
    
    DEPENDS ${yaml-cpp_DEPENDENCIES}
    
    LIST_SEPARATOR |
    CMAKE_ARGS ${yaml-cpp_ARGS})
