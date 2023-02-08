#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

moved=""
if [[ -d packages ]]; then
    moved=1
    mkdir -p old_packages
    mv -f packages/*.exe old_packages/
    mv -f packages/*.zip old_packages/
    mv -f packages/*.dmg old_packages/
    rm -rf packages
fi


#
#
# Main build script for mrv2 using Docker.
# We build binaries on a Rocky Linux 8 image.
#

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


if [[ $moved == 1 ]]; then
    mv old_packages/* packages/
fi
