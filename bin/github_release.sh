#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This file is to prepare a new tagged release on gihub.
# It will pull and push the latest changes, create the local and remote
# tag (if already present aski you if you want to continue), it will
# stop and remove all docker containers, images and logs on Linux.
#
# It should be run from the main root of the mrv2 tree like:
#   ./bin/github_release.sh
#


#
# Read some auxiliary functions
#

. ./etc/build_dir.sh


if [[ $KERNEL == *Linux* ]]; then
    . ./etc/prepare_release.sh
    ./bin/clean_docker.sh
    echo "Run the docker build..."
    ./runme_docker.sh
else
    ./runme.sh dist
    ./runmeq.sh -t mo
    ./runmeq.sh -t package
fi
