#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



extract_version()
{
    major=`cat cmake/version.cmake | grep -Po '(VERSION_MAJOR\s)\d' | sed -e 's/VERSION_MAJOR\s*//'`
    minor=`cat cmake/version.cmake | grep -Po '(VERSION_MINOR\s)\d' | sed -e 's/VERSION_MINOR\s*//'`
    patch=`cat cmake/version.cmake | grep -Po '(VERSION_PATCH\s)\d' | sed -e 's/VERSION_PATCH\s*//'`
    export mrv2_VERSION="${major}.${minor}.${patch}"
}


#
# SOME DEFINES
#
GIT_EXECUTABLE=git
DOCKER_EXECUTABLE=docker

extract_version


#
# Pull last changes
#
echo "Switch to main and pull changes"
${GIT_EXECUTABLE} switch main && ${GIT_EXECUTABLE} pull

echo "Now push any commited changes"
${GIT_EXECUTABLE} push

#
# Delete local tag if available
#
echo "Remove local tag v${mrv2_VERSION}"
${GIT_EXECUTABLE} tag -d v${mrv2_VERSION}

#
# Delete remote tag if available
#
echo "Remove remote tag v${mrv2_VERSION}"
${GIT_EXECUTABLE} push --delete origin v${mrv2_VERSION}



#
# Mark current repository with a new tag
#
echo "Create local tag v${mrv2_VERSION}"
${GIT_EXECUTABLE} tag v${mrv2_VERSION}

#
# Send new tag to repository
#
echo "Create remote tag v${mrv2_VERSION}"
${GIT_EXECUTABLE} push origin v${mrv2_VERSION}

#
# Stop all running containers
#
echo "Stopping all containers..."
${DOCKER_EXECUTABLE} stop $(${DOCKER_EXECUTABLE} ps -aq)

#
# Remove all containers
#
echo "Removing all containers..."
${DOCKER_EXECUTABLE} rm $(${DOCKER_EXECUTABLE} ps -aq)

#
# Remove the images if present
#
echo "Removing all images..."
${DOCKER_EXECUTABLE} rmi $(${DOCKER_EXECUTABLE} images -q)

#
# Remove all volumes
#
echo "Removing all images..."
${DOCKER_EXECUTABLE} volume prune -f

echo "Run the docker build..."
runme_docker.sh
