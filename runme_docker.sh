#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



#
#
# Main build script for mrv2 using Docker.
# We build binaries on a Rocky Linux 8 image.
#

#
# Clear packages directory
#
rm -rf $PWD/packages

#
# Build the image
#
docker build -t mrv2_builder .

#
# Run the compile and package extraction
#
docker run -v ${PWD}/packages:/packages \
       --name mrv2_build_$(date "+%s") \
       mrv2_builder
