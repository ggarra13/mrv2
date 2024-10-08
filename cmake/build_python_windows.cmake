# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



message(STATUS "Python_COMMAND=${Python_COMMAND}")
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
message(STATUS "PATH ENV 1=$ENV{PATH}")
message(STATUS "PATH     1=${PATH}")
string(REPLACE "|" ";" $ENV{PATH} "${PATH}")
message(STATUS "PATH ENV 2=$ENV{PATH}")

set(ROOT_DIR ${CMAKE_INSTALL_PREFIX}/../deps/Python/src/Python)

if (Python_COMMAND STREQUAL "build")
    set(CMD PCbuild\\build.bat -q -p ${Python_PLATFORM} --pgo )
elseif(Python_COMMAND STREQUAL "install")
    set(CMD python.bat PC\\layout --precompile --preset-default  --copy "${CMAKE_INSTALL_PREFIX}/bin/")
else()
    message(FATAL_ERROR "Unknown Python_COMMAND ${Python_COMMAND}!")
endif()

message(STATUS "Running: ${CMD} in ${ROOT_DIR}...")

execute_process(
    COMMAND ${CMD}
    WORKING_DIRECTORY ${ROOT_DIR}
    ERROR_VARIABLE BAT_CMD_ERROR
    OUTPUT_VARIABLE BAT_CMD_OUTPUT
    ECHO_ERROR_VARIABLE 
    ECHO_OUTPUT_VARIABLE 
    COMMAND_ERROR_IS_FATAL ANY
)
