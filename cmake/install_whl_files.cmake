# install_whl_files.cmake

# Check if the WHL_DIRECTORY variable is provided
if(NOT DEFINED PYTHON_EXECUTABLE)
    message(FATAL_ERROR "PYTHON_EXECUTABLE is not defined. Please specify the path to the python executable using -DPYTHON_EXECUTABLE=<path/to/python>")
endif()

# Check if the WHL_DIRECTORY variable is provided
if(NOT DEFINED WHL_DIRECTORY)
    message(FATAL_ERROR "WHL_DIRECTORY is not defined. Please specify the directory containing .whl files using -DWHL_DIRECTORY=<directory>")
endif()

execute_process(
    COMMAND sw_vers -productVersion
    OUTPUT_VARIABLE MACOS_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Find all .whl files in the specified directory
message(STATUS "Checking for wheel files ${WHL_DIRECTORY}/*.whl")
file(GLOB whl_files "${WHL_DIRECTORY}/*.whl")

# Install the found .whl files
foreach(whl_file ${whl_files})
    
    # Get the base name of the .whl file (without the path)
    get_filename_component(whl_filename ${whl_file} NAME)

    # Install the .whl file to the desired installation location
    message( STATUS "Running pip install - ${whl_filename} - ..." )
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} -m pip install ${whl_filename}
	WORKING_DIRECTORY ${WHL_DIRECTORY}
    )
endforeach()
