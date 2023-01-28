#!/bin/bash

# Clear out the /build and /release directory
rm -rf /build/*
rm -rf /release/*

# Re-pull the repository
git fetch && \
    BUILD_VERSION=$(git describe --tags $(git rev-list --tags --max-count=1)) && \
    git checkout ${BUILD_VERSION}

# Run the build
./runme.sh

# Create the .deb, .rpm and tar.gz packages
./runmeq.sh -t package

# Move it to the release volume
mv BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.deb    /release
mv BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.rpm    /release
mv BUILD-Linux-64/Release/mrv2/src/mrv2-build/*.tar.gz /release
