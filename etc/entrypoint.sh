#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

echo "------------------------------------"
echo "Starting docker run as user $USER..."
echo "------------------------------------"
#
# This script is a docker ENTRYFILE used to clone, build and extract the
# packages that cmake created into their own directory.
#
TAG=$1
REPO=https://github.com/ggarra13/mrv2.git

echo "Cloning tag ${TAG}..."
git clone $REPO --depth 1 --branch ${TAG}

cd mrv2

#
# Download a latest cmake and install it in staging area
#
chmod a+x ./etc/install_cmake.sh
./etc/install_cmake.sh

#
# Run the build and package it.
#
# We build cmake as the one in Rocky Linux is too old for OpenColorIO.
# We always build python to make sure we ship the save version across OSes.
#
./runme.sh -t package


#
# Finally, copy the packages over
#
echo "We have the following packages in the ./packages directory"
echo "Current dir is $PWD"
echo "Moving packages to /packages volume"
mv ./packages/*.deb    /packages/
mv ./packages/*.rpm    /packages/
mv ./packages/*.tar.gz /packages/
echo "------------------------------"
ls /packages/
echo "------------------------------"
