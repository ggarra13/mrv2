
function(find_meson_executable TARGET)
    # 1. Initialize local variables to capture logic results
    set(_LOCAL_MESON "")
    set(_LOCAL_PYTHONPATH "")

    if(NOT BUILD_PYTHON)
        find_program(_LOCAL_MESON
	    NAMES
	    meson meson.exe
	    /opt/homebrew/bin
	    /usr/local/bin
	)
        if(NOT _LOCAL_MESON)
            message(FATAL_ERROR "Meson build system not found!")
        endif()
    else()
	if(WIN32)
            set(_LOCAL_MESON "${CMAKE_INSTALL_PREFIX}/Bin/Scripts/meson")
        else()
            set(_LOCAL_MESON "${CMAKE_INSTALL_PREFIX}/bin/meson")
            set(_LOCAL_PYTHONPATH "PYTHONPATH=${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION}:${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION}/site-packages")
        endif()
    endif()

    # 2. Lift the final values to PARENT_SCOPE
    set(MESON_EXECUTABLE "${_LOCAL_MESON}" PARENT_SCOPE)
    set(${TARGET}_PYTHONPATH "${_LOCAL_PYTHONPATH}" PARENT_SCOPE)
    
    message(STATUS "Found Meson: ${_LOCAL_MESON}")
endfunction()
