/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



#
# This script is a docker ENTRYFILE used to build and extract the packages that
# cmake created into their own directory.
#

#
# Run the build.  Use -G Ninja for faster but not so descriptive builds
#

CPU_CORES=$(awk '/^processor/ {++n} END {print n+1}' /proc/cpuinfo)

echo "Building with ${CPU_CORES} cores..."

./runme.sh -G 'Unix Makefiles' -j ${CPU_CORES}

echo "Packaging with ${CPU_CORES} cores..."

#
# Create the .deb, .rpm and tar.gz packages
#
./runmeq.sh -j ${CPU_CORES} -t package

#
# Finally, copy the packages over
#
echo "Moving packages to /packages volume"
mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.deb    /packages/
mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.rpm    /packages/
mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.tar.gz /packages/
echo "We created the following packages in the /packages directory"
ls /packages/
echo "----------------------------"
