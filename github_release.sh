#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Read some auxiliary functions
#
. ./etc/prepare_release.sh


DOCKER_EXECUTABLE=docker

#
# Stop all running containers
#
echo "Stopping all containers..."
containers=`${DOCKER_EXECUTABLE} ps -aq`
if [[ $containers != "" ]]; then
    ${DOCKER_EXECUTABLE} stop $containers

    #
    # Remove all containers
    #
    echo "Removing all containers..."
    ${DOCKER_EXECUTABLE} rm $contatiners
fi

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
