cmake_minimum_required(VERSION 3.19)

project(tlRender-external-tests)

enable_testing()

foreach(EXTERNAL_DEP ${TLRENDER_EXTERNAL_DEPS})
    if(EXISTS ${CMAKE_SOURCE_DIR}/${EXTERNAL_DEP}Test)
        add_subdirectory(${EXTERNAL_DEP}Test)
    endif()
endforeach()
