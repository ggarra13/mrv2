# install_whl_files.cmake

# Check if the WHL_DIRECTORY variable is provided
if(NOT DEFINED WHL_DIRECTORY)
    message(FATAL_ERROR "WHL_DIRECTORY is not defined. Please specify the directory containing .whl files using -DWHL_DIRECTORY=<directory>")
endif()

# Check if the WHL_DIRECTORY variable is provided
if(NOT DEFINED PYTHON_EXECUTABLE)
    message(FATAL_ERROR "PYTHON_EXECUTABLE is not defined. Please specify the path to the python executable using -DPYTHON_EXECUTABLE=<path/to/python>")
endif()

# Install the wheel python module
message( STATUS "Running pip install wheel..." )
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -m pip install wheel
)
    
# Find all .whl files in the specified directory
file(GLOB whl_files "${WHL_DIRECTORY}/*.whl")

# Install the found .whl files
foreach(whl_file ${whl_files})
    # Get the base name of the .whl file (without the path)
    get_filename_component(whl_filename ${whl_file} NAME)

    # Install the .whl file to the desired installation location
    message( STATUS "Running pip install ${whl_file}..." )
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} -m pip install ${whl_file}
    )
endforeach()
