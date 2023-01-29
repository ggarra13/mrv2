#!/usr/bin/env bash

#
# This script is a docker ENTRYFILE used to extract the packages that
# cmake created into their own directory.
#

echo "Moving packages to /release volume"
rm -rf /packages/*

#
# Run the build.  Use -G Ninja for faster but not so interactive builds
#
./runme.sh -G 'Unix Makefiles'

# Create the .deb, .rpm and tar.gz packages
./runmeq.sh -t package


mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.deb    /packages/
mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.rpm    /packages/
mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.tar.gz /packages/
echo "We created the following packages in the /package directory"
ls /packages/
echo "----------------------------"
