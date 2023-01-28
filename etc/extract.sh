#!/usr/bin/env bash

#
# This script is a docker ENTRYFILE used to extract the packages that
# cmake created into their own directory.
#

echo "Moving packages to /release volume"
rm -rf /package/*

mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.deb    /package/
mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.rpm    /package/
mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.tar.gz /package/
echo "We created the following packages in the /package directory"
ls /package/
echo "----------------------------"
