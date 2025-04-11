# Function to convert path to Msys2
function(convert_path_for_msys2 IN_PATH OUT_PATH)
    # Split the path at the drive letter (if present)
    string(REGEX REPLACE "^([A-Z]):/" "/\\1/" INTERMEDIATE_PATH "${IN_PATH}")

    # Convert backslashes to forward slashes
    string(REPLACE "\\" "/" INTERMEDIATE_PATH "${INTERMEDIATE_PATH}")

    # Return the converted path
    set(${OUT_PATH} "${INTERMEDIATE_PATH}" PARENT_SCOPE)
endfunction()
