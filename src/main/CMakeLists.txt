# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


set(HEADERS )
set(SOURCES main.cpp )

files_to_absolute_paths()
set( PO_SOURCES ${PO_FILES} ${PO_SOURCES} PARENT_SCOPE)

if (WIN32)
    enable_language("RC")
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/app.rc")
    list( APPEND SOURCES ${APP_ICON_RESOURCE_WINDOWS} )
endif()

set(LIBRARIES )

if(MRV2_PYBIND11)
    list(APPEND LIBRARIES mrvPy)
endif()

list(APPEND LIBRARIES mrvApp)

if(TLRENDER_VK)
    list(APPEND LIBRARIES ${FLTK_vk_LIBRARY})
endif()

list(APPEND LIBRARIES ${FLTK_LIBRARIES} ${Intl_LIBRARIES})


if( APPLE )
    set(OSX_FRAMEWORKS "-framework IOKit")
    list(APPEND LIBRARIES ${OSX_FRAMEWORKS})
endif()

if(UNIX AND NOT APPLE)
    # Needed for python3.11 on Rocky and Ubuntu 20.04
    list(APPEND LIBRARIES "util" "c" )
endif()

add_executable(mrv2 WIN32  ${SOURCES} ${HEADERS})


target_link_libraries(mrv2 PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories(mrv2 BEFORE PUBLIC ${CMAKE_INSTALL_PREFIX}/lib /usr/local/lib )

set(PRUNE_SYMBOLS )
set(EXCLUDE_LIBRARIES )

#
# For Vulkan development we turn off strip symbols
#
# if(WIN32)
#     set(PRUNE_SYMBOLS "/OPT:REF" "/OPT:ICF")
# elseif(APPLE)
#     set(PRUNE_SYMBOLS "-Wl,-dead_strip")
# else(UNIX)
#     set(PRUNE_SYMBOLS "-Wl,--gc-sections")
#     set(EXCLUDE_LIBRARIES "-Wl,--exclude-libs=ALL")
# endif()
# target_link_options(mrv2 PUBLIC ${EXCLUDE_LIBRARIES} PRIVATE ${PRUNE_SYMBOLS})

install(TARGETS mrv2
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION lib COMPONENT libraries
    ARCHIVE DESTINATION lib COMPONENT libraries )


#
# Handle translations
#
include( ../../cmake/translations.cmake )

#
# Handle documentation
#
include( ../../cmake/documentation.cmake )


#
# Handle install of DLLs for mrv2
#
# if (WIN32)
#     add_custom_command(TARGET mrv2 POST_BUILD
# 	COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:mrv2> $<TARGET_RUNTIME_DLLS:mrv2>
# 	COMMAND_EXPAND_LISTS
#     )
# endif()


#
# Extract python version
#
string( REGEX MATCH "^[0-9]+\\.[0-9]+" MRV2_PYTHON_VERSION "${Python_VERSION}" )

if(BUILD_PYTHON)
    if(NOT MRV2_PYTHON_VERSION OR MRV2_PYTHON_VERSION STREQUAL "")
	message(FATAL_ERROR "Could not detect python version from "
	    "${Python_VERSION}")
    endif()
	
    if (UNIX)
	set( MRV2_PYTHON_LIBS ${CMAKE_INSTALL_PREFIX}/lib/python${MRV2_PYTHON_VERSION} )
    else()
	set( MRV2_PYTHON_LIBS ${CMAKE_INSTALL_PREFIX}/bin/Lib )
    endif()
    
    if (MRV2_PYFLTK)
	# Get rid of the pyFLTK demos as they don't work properly on an
	# embedded python
	file( REMOVE_RECURSE ${MRV2_PYTHON_LIBS}/site-packages/fltk14/test )
    endif()

    if (UNIX)
	install( DIRECTORY ${MRV2_PYTHON_LIBS}
            DESTINATION lib
            COMPONENT applications )
    elseif(WIN32)
	install( DIRECTORY ${MRV2_PYTHON_LIBS}
            DESTINATION bin
            COMPONENT applications )
    endif()
endif()
	

#
# Install python's requests library
#
if (MRV2_PYBIND11)
    if (NOT EXISTS "${MRV2_PYTHON_LIBS}/site-packages/requests")
	execute_process( COMMAND ${CMAKE_COMMAND} -E env "DYLD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib" ${PYTHON_EXECUTABLE} -m pip install --target "${MRV2_PYTHON_LIBS}/site-packages" requests --no-cache-dir )
    endif()
endif()

if(TLRENDER_NDI)
    # NDI libraries have two names: Advanced and normal.
    # We use a glob to simplify and also for potential future development
    # of NDI's SDK.
    if (WIN32)
	if (TLRENDER_NDI_SDK MATCHES ".*Advanced.*")
	    # Advanced lib is ${TLRENDER_NDI_SDK}/bin/x64/Processing.NDI.Lib.Advanced.x64.dll
	    file( GLOB MRV2_LIB_DLLS ${TLRENDER_NDI_SDK}/bin/x64/Processing.NDI.Lib.Advanced.x64.dll )
	else()
	    # Normal lib is ${TLRENDER_NDI_SDK}/bin/x64/Processing.NDI.Lib.x64.dll
	    file( GLOB MRV2_LIB_DLLS ${TLRENDER_NDI_SDK}/bin/x64/Processing.NDI.Lib.x64.dll )
	endif()
	install( FILES  ${MRV2_LIB_DLLS}
	    DESTINATION bin
	    COMPONENT applications )
    elseif (APPLE)
	file( GLOB MRV2_LIB_DLLS ${TLRENDER_NDI_SDK}/lib/macOS/*.dylib* )
	install( FILES ${MRV2_LIB_DLLS}
	    DESTINATION lib
	    COMPONENT applications )
    elseif(UNIX)
	file( GLOB MRV2_LIB_DSOS ${TLRENDER_NDI_SDK}/lib/x86_64-linux-gnu/*.so* )
	install( FILES ${MRV2_LIB_DSOS}
	    DESTINATION lib
	    COMPONENT applications )
    endif()
endif()

    
if ( APPLE )

    if(MRV2_PYBIND11)
	# Install the main python executable
	install( FILES
	    ${PYTHON_EXECUTABLE}
	    DESTINATION bin/
            PERMISSIONS 
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
	    COMPONENT applications )
    endif()
    
    # Install all .dylibs in CMAKE_INSTALL_PREFIX / lib
    file( GLOB MRV2_DYLIBS
	${CMAKE_INSTALL_PREFIX}/lib/*.dylib*
    )
    install( FILES ${MRV2_DYLIBS}
	DESTINATION lib
	COMPONENT applications )

    # Install all .frameworks in CMAKE_INSTALL_PREFIX / lib
    file( GLOB MRV2_FRAMEWORKS
	${CMAKE_INSTALL_PREFIX}/lib/*.framework* )

    foreach( framework ${MRV2_FRAMEWORKS} )
	install( DIRECTORY ${framework}
	    DESTINATION lib
	    COMPONENT applications )
    endforeach()
    
elseif ( UNIX )

    if(MRV2_PYBIND11)
	# Install the main python executable
	install( FILES
	    ${PYTHON_EXECUTABLE}
	    DESTINATION bin/
            PERMISSIONS 
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
	    COMPONENT applications )
    endif()
    
    # Install all .so in CMAKE_INSTALL_PREFIX/lib
    file( GLOB MRV2_LIB_DSOS "${CMAKE_INSTALL_PREFIX}/lib/*.so*" )
    install( FILES ${MRV2_LIB_DSOS}
	DESTINATION lib
	COMPONENT applications )
  
    # For RedHat and Rocky Linux, we must install also lib64
    file( GLOB MRV2_LIB64_DSOS "${CMAKE_INSTALL_PREFIX}/lib64/*.so*" )
    install( FILES ${MRV2_LIB64_DSOS}
      DESTINATION lib64
      COMPONENT applications )
  
elseif(WIN32)
    #
    # Handle python DLL and libs and TCL
    #
    if(MRV2_PYBIND11)

	if (MRV2_PYFLTK)
	    # Get rid of the pyFLTK demos as they don't work properly on an
	    # embedded python
	    file( REMOVE_RECURSE "${CMAKE_INSTALL_PREFIX}/bin/Lib/site-packages/fltk14/test" )
	endif()
    
	# Install the main python executable
	install( FILES "${PYTHON_EXECUTABLE}"
	    DESTINATION bin/
	    COMPONENT applications )
	
	# Install all .dll in CMAKE_INSTALL_PREFIX/bin
	file( GLOB MRV2_PYLibs "${CMAKE_INSTALL_PREFIX}/bin/Lib/*" )
	set( MRV2_PYDLLs "${CMAKE_INSTALL_PREFIX}/bin/DLLs" )
	set( MRV2_PYTCL "${CMAKE_INSTALL_PREFIX}/bin/tcl"  )
	set( MRV2_TK_LIBS
	    "${CMAKE_INSTALL_PREFIX}/bin/Lib/tkinter" 
	    "${CMAKE_INSTALL_PREFIX}/bin/Lib/idlelib" 
	    "${CMAKE_INSTALL_PREFIX}/bin/Lib/turtledemo" )

	set(MRV2_PYLIB_DIRECTORIES "")
	set(MRV2_PYLIB_FILES "")

	foreach(item ${MRV2_PYLibs})
	    set(_ignore FALSE)
	    # Skip Tk libs and demos
	    foreach(ignore ${MRV2_TK_LIBS})
		if("${item}" MATCHES "${ignore}")
		    set(_ignore TRUE)
		    break()
		endif()
	    endforeach()
	    if ("${_ignore}")
		continue()
	    endif()
	    if (IS_DIRECTORY "${item}")
		# Add directory to directories list
		list(APPEND MRV2_PYLIB_DIRECTORIES "${item}")
	    else()    
		list(APPEND MRV2_PYLIB_FILES "${item}")
	    endif()
	endforeach()

	
	install( DIRECTORY ${MRV2_PYLIB_DIRECTORIES}
	    DESTINATION bin/Lib
	    COMPONENT applications )

	install( FILES ${MRV2_PYLIB_FILES}
	    DESTINATION bin/Lib
	    COMPONENT applications )

	install( DIRECTORY ${MRV2_PYDLLs}
	    DESTINATION bin/
	    COMPONENT applications )

	install( DIRECTORY ${MRV2_PYTCL}
	    DESTINATION bin/
	    COMPONENT python_tk )
	
	install( DIRECTORY ${MRV2_TK_LIBS}
	    DESTINATION bin/Lib
	    COMPONENT python_tk )
    endif()
        
    # For Python, Poco, USD, FLTK, NDI, MSVC runtimes and FFmpeg
    file(GLOB MRV2_DLLS "${CMAKE_INSTALL_PREFIX}/bin/*.dll"  )

    install(FILES ${MRV2_DLLS}
	DESTINATION bin/
	COMPONENT applications )
    
	    
endif()


if ( NOT WIN32 )
    # Configure environment.sh with python path
    configure_file(${CMAKE_SOURCE_DIR}/bin/environment.sh.in
	${CMAKE_INSTALL_PREFIX}/bin/environment.sh @ONLY)
    
    install( FILES
	${CMAKE_SOURCE_DIR}/bin/mrv2.sh
	${CMAKE_INSTALL_PREFIX}/bin/environment.sh
	DESTINATION bin
        PERMISSIONS 
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
	COMPONENT applications )

    if(BUILD_PYTHON)
    	# Configure python.sh with python version
	configure_file(${CMAKE_SOURCE_DIR}/bin/python.sh.in
	${CMAKE_INSTALL_PREFIX}/bin/python.sh @ONLY)

        # Configure pip.sh with python version
	configure_file(${CMAKE_SOURCE_DIR}/bin/pip.sh.in
	    ${CMAKE_INSTALL_PREFIX}/bin/pip.sh @ONLY)
    
	file(CHMOD ${CMAKE_INSTALL_PREFIX}/bin/python.sh
            PERMISSIONS 
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE )
    
	file(CHMOD ${CMAKE_INSTALL_PREFIX}/bin/pip.sh
            PERMISSIONS 
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE )
    
	install( FILES
	    ${CMAKE_INSTALL_PREFIX}/bin/pip.sh
	    ${CMAKE_INSTALL_PREFIX}/bin/python.sh
	    DESTINATION bin
            PERMISSIONS 
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
	    COMPONENT applications )
    endif()

    if(APPLE)
	install( FILES
	${CMAKE_SOURCE_DIR}/bin/install_dmg.sh
	DESTINATION bin
        PERMISSIONS 
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
	COMPONENT applications )
    endif()
    
    if(TLRENDER_USD)
	install( DIRECTORY
	    ${CMAKE_INSTALL_PREFIX}/lib/usd
	    DESTINATION lib
	    COMPONENT applications)
    endif()
else()
    if(TLRENDER_USD)
	install( DIRECTORY
	    ${CMAKE_INSTALL_PREFIX}/bin/usd
	    DESTINATION bin
	    COMPONENT applications)
    endif()
endif()


#
# USD dependencies
#
if(TLRENDER_USD)
    install( DIRECTORY
        "${CMAKE_INSTALL_PREFIX}/plugin"
        "${CMAKE_INSTALL_PREFIX}/libraries"
        DESTINATION .
        COMPONENT applications)
endif()

