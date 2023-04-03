#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This file will stop and remove all docker containers, images and logs.
#

bin/clean_containers.sh

DOCKER_EXECUTABLE=docker

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
