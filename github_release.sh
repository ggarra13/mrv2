#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This file is to prepare a new tagged release on gihub.
# It will pull and push the latest changes, remote the local and remote
# tag if already present (asking you if you want to continue), it will
# stop and remove all docker containers, images and logs.
#


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
    ${DOCKER_EXECUTABLE} rm $containers
fi

#
# Remove the images if present
#
echo "Removing all images..."
images=`${DOCKER_EXECUTABLE} images -q`
if [[ $images != "" ]]; then
    ${DOCKER_EXECUTABLE} rmi ${images}
fi

#
# Remove all docker caches
#

echo "Clear all docker caches..."
${DOCKER_EXECUTABLE} builder prune

echo "Run the docker build..."
runme_docker.sh
