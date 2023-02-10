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
. ./etc/clean_docker.sh
echo "Run the docker build..."
runme_docker.sh
