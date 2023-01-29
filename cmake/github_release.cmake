#!/usr/bin/env cmake

#
# This script is used on Linux to clear (if needed) and
# mark a new tag release based on
#    cmake/version.cmake
#
# and then run a docker build.
#

include( cmake/version.cmake )

find_package( Git REQUIRED )

#
# Delete local tag if available
#
message( STATUS "Remove packages directory" )
execute_process( COMMAND rm -rf packages )

#
# Delete local tag if available
#
message( STATUS "Remove local tag v${mrv2_VERSION}" )
execute_process( COMMAND ${GIT_EXECUTABLE} tag -d v${mrv2_VERSION} )

#
# Delete remote tag if available
#
message( STATUS "Remove remote tag v${mrv2_VERSION}" )
execute_process( COMMAND ${GIT_EXECUTABLE} push --delete origin v${mrv2_VERSION} )


#
# Mark current repository with a new tag
#
message( STATUS "Create local tag v${mrv2_VERSION}" )
execute_process( COMMAND ${GIT_EXECUTABLE} tag v${mrv2_VERSION} )

#
# Send new tag to repository
#
message( STATUS "Create remote tag v${mrv2_VERSION}" )
execute_process( COMMAND ${GIT_EXECUTABLE} push origin v${mrv2_VERSION} )

#
# Start runme_docker.sh script
#
message( STATUS "Run docker build with tag v${mrv2_VERSION}" )
execute_process( COMMAND runme_docker.sh )
