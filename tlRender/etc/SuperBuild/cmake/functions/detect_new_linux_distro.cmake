function(detect_new_linux_distro OUT_NEW_DISTRO)
    set(${OUT_NEW_DISTRO} FALSE PARENT_SCOPE)
    
    # 1. Detect Linux Distribution and Version
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	# Read the standard OS identification file
	file(READ "/etc/os-release" OS_RELEASE_CONTENTS)

	# Extract ID (e.g., ubuntu, rocky)
	string(REGEX MATCH "NAME=\"?([a-zA-Z0-9_]+)\"?" _MATCH "${OS_RELEASE_CONTENTS}")
	set(LINUX_DISTRO_ID ${CMAKE_MATCH_1})
	
	# Extract Version (e.g., 25.04, 8.10)
	string(REGEX MATCH "VERSION_ID=\"?([0-9.]+)\"?" _MATCH "${OS_RELEASE_CONTENTS}")
	set(LINUX_DISTRO_VERSION ${CMAKE_MATCH_1})
    endif()

    # 2. Detect SCL (Software Collections)
    # SCL sets the 'X_SCLS' environment variable when active
    if(DEFINED ENV{X_SCLS} AND NOT "$ENV{X_SCLS}" STREQUAL "")
	set(IS_RUNNING_SCL TRUE)
	set(${OUT_NEW_DISTRO} FALSE PARENT_SCOPE)
	message(STATUS "SCL Detected: $ENV{X_SCLS}")
    else()
	set(IS_RUNNING_SCL FALSE)
    endif()

    # 3. Apply Logic
    if(LINUX_DISTRO_ID STREQUAL "Ubuntu" AND LINUX_DISTRO_VERSION VERSION_GREATER_EQUAL "25.0")
	set(${OUT_NEW_DISTRO} TRUE PARENT_SCOPE)
	message(STATUS ">> Detected Target: Ubuntu 25+ (Version: ${LINUX_DISTRO_VERSION}) ${OUT_NEW_DISTRO}=${${OUT_NEW_DISTRO}}")

    elseif(LINUX_DISTRO_ID STREQUAL "Rocky" AND LINUX_DISTRO_VERSION VERSION_EQUAL "8.10")
	set(${OUT_NEW_DISTRO} FALSE PARENT_SCOPE)
    else()
	message(STATUS ">> Detected Other: ${LINUX_DISTRO_ID} ${LINUX_DISTRO_VERSION}")
	set(${OUT_NEW_DISTRO} FALSE PARENT_SCOPE)
    endif()
endfunction()
