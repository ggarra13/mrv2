#!/usr/bin/env bash

#
#
# Main build script for mrv2 using Docker.
# We build binaries on a Rocky Linux 8 image.
#

docker build -t mrv2_builder .
docker run -v ${PWD}/packages:/packages \
       --name mrv2_build_$(date "+%s") \
       mrv2_builder
