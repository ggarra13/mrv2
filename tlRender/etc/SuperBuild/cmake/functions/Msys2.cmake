# Function to convert path to Msys2
function(convert_path_for_msys2 IN_PATH OUT_PATH)
    # Split the path at the drive letter (if present)
    string(REGEX REPLACE "^([A-Z]):/" "/\\1/" INTERMEDIATE_PATH "${IN_PATH}")

    # Convert backslashes to forward slashes
    string(REPLACE "\\" "/" INTERMEDIATE_PATH "${INTERMEDIATE_PATH}")

    # Return the converted path
    set(${OUT_PATH} "${INTERMEDIATE_PATH}" PARENT_SCOPE)
endfunction()

if(WIN32)
    find_package(Msys REQUIRED)
    if($ENV{ARCH} MATCHES ".*arm64.*" OR $ENV{ARCH} MATCHES ".*aarch64.*")
	if (EXISTS "$ENV{TEMP}/msys2_cmd.bat")
	    set(MRV2_MSYS_CMD "$ENV{TEMP}/msys2_cmd.bat")
	else()
	    set(MRV2_MSYS_CMD
		${MSYS_CMD}
		-use-full-path
		-defterm
		-no-start
		-here)
	endif()
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
