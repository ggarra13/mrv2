#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Turn on exit on error
#
set -o pipefail -e


#
# This script is a docker ENTRYFILE used to build and extract the packages that
# cmake created into their own directory.
#

. ./etc/build_dir.sh

CPU_CORES=$(awk '/^processor/ {++n} END {print n+1}' /proc/cpuinfo)
echo "Building with ${CPU_CORES} cores..."

#
# Run the build.  Use -G Ninja for faster but not so descriptive builds
#
#./runme.sh -G 'Unix Makefiles' -j ${CPU_CORES}
./runme.sh -G Ninja -j ${CPU_CORES}

#
# Create the translation files (.mo files in share/locale/LC_MESSAGES dir).
#
echo "Creating translation files..."
./runmeq.sh -j ${CPU_CORES} -t mo

#
# Create the .deb, .rpm and tar.gz packages
#
echo "Packaging with ${CPU_CORES} cores..."

./runmeq.sh -j ${CPU_CORES} -t package

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
