# Function to convert path to Msys2
function(convert_path_for_msys2 IN_PATH OUT_PATH)
    # Split the path at the drive letter (if present)
    string(REGEX REPLACE "^([A-Z]):/" "/\\1/" INTERMEDIATE_PATH "${IN_PATH}")

    # Convert backslashes to forward slashes
    string(REPLACE "\\" "/" INTERMEDIATE_PATH "${INTERMEDIATE_PATH}")

    # Return the converted path
    set(${OUT_PATH} "${INTERMEDIATE_PATH}" PARENT_SCOPE)
endfunction()


find_package(Msys REQUIRED)
if(WIN32)
    if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "ARM64|AArch64")
	set(MRV2_MSYS_CMD
	    ${MSYS_CMD})
    else()
	set(MRV2_MSYS_CMD
	    ${MSYS_CMD}
	    -use-full-path
	    -defterm
	    -no-start
	    -here)
    endif()
else()
    set(MRV2_MSYS_CMD)
endif()
