#!/usr/bin/env bash

echo "Moving packages to /release volume"
rm -rf /release/*

mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.deb    /release
mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.rpm    /release
mv /mrv2/BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.tar.gz /release
echo "ls on /release volume"
ls /release
echo "----------------------------"
