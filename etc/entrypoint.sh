#!/usr/bin/env bash

#
# This script is a docker ENTRYFILE used to build and extract the packages that
# cmake created into their own directory.
#

echo "Moving packages to /release volume"
rm -rf /packages/*

#
# Run the build.  Use -G Ninja for faster but not so descriptive builds
#

CPU_CORES=$(awk '/^processor/ {++n} END {print n+1}' /proc/cpuinfo)
./runme.sh -G 'Unix Makefiles' -j ${CPU_CORES}

#
# Create the .deb, .rpm and tar.gz packages
#
./runmeq.sh -t package

#
# List all dependencies to see if any are missing
#
echo "DSOs needed:"
ldd /mrv2/BUILD-Linux-64/Release/install/bin/mrv2

echo "DSOs installed:"
ls /mrv2/BUILD-Linux-64/Release/install/lib

#
# Finally, copy the packages over
#
mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.deb    /packages/
mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.rpm    /packages/
mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.tar.gz /packages/
echo "We created the following packages in the /packages directory"
ls /packages/
echo "----------------------------"
