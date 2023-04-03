#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

echo "----------------------"
echo "Starting docker run..."
echo "----------------------"
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
# Get the CPU cores
#
CPU_CORES=$(awk '/^processor/ {++n} END {print n+1}' /proc/cpuinfo)

#
# Run the build and package it.
#
echo "Building with ${CPU_CORES} cores..."
./runme.sh -G 'Unix Makefiles' -j ${CPU_CORES} -t package


#
# Finally, copy the packages over
#
echo "We have the following packages in the ./packages directory"
echo "Moving packages to /packages volume"
mv ./packages/*.deb    /packages/
mv ./packages/*.rpm    /packages/
mv ./packages/*.tar.gz /packages/
echo "------------------------------"
ls /packages/
echo "------------------------------"
