#!/usr/bin/env bash

#
#
# Main build script for mrv2 using Docker.
# We build binaries on an Ubuntu 16.04 LTS image.
#

docker build -t mrv2_builder .
docker run -v ${PWD}/release:release --name mrv2_build_$(date "+%s") mrv2_builder
