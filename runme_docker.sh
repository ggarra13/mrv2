#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
#
# Main build script for mrv2 using Docker.
# We build binaries on a Rocky Linux 8 image.
# We build the latest tag released by default.  If you want to build the
# current HEAD change the Dockerfile.
#

export LATEsT=0
for i in $@; do
    case $i in
	latest)
	    export LATEST=1
	    shift
	    ;;
    esac
done

#
# Build the image
#
if [[ $LATEST == 1 ]]; then
    echo "Docker will build the latest HEAD of repository."
    docker build -t mrv2_builder -f Dockerfile.latest .
else
    echo "Docker will build the latest TAG of repository."
    docker build -t mrv2_builder .
fi

#
# Run the compile and package extraction
#
docker run -v ${PWD}/packages:/packages \
       --name mrv2_build_$(date "+%s") \
       mrv2_builder
