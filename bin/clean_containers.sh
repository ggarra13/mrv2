#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This file will stop and remove all docker containers, images and logs.
#
# You must run it from the root of the mrv2 project.
#


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

