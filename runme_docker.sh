#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
#
# Main build script for mrv2 using Docker.
# We build binaries on a Rocky Linux 8 image.
# We build the latest tag released by default.
#
# This script, if run without parameters, will build the latest release.
# If run as:
#
# $ runme_docker.sh main
#
# It will build the main HEAD.
#
# If run with any other parameter, it will assume it is a tag or branch name
# and it will build it.
#
#

#
# Turn on exit on error
#
set -o pipefail -e

export WHAT=release
for i in $@; do
    case $i in
	-h|-help|--help)
	    echo "$0 - Run a docker compilation of mrv2"
	    echo ""
	    echo "$0 [main|release|branch|tag]"
	    echo
	    echo "The default behavior is release, which builds the latest"
	    echo "tagged release."
	    echo "Option main will build the latest head from branch main."
	    echo "ANy other option is considered a branch or tag."
	    exit 0
	    ;;
	latest|HEAD|main)
	    export WHAT=main
	    shift
	    ;;
	release)
	    export WHAT=release
	    shift
	    ;;
	*)
	    export WHAT=$i
	    shift
	    ;;
    esac
done

export TAG=$WHAT
export TYPE=branch
if [[ $WHAT == release ]]; then
    export TAG=`git ls-remote --tags --refs | tail -n1 | cut -d/ -f3`
elif [[ $WHAT != main ]]; then
    export has_tag=`git ls-remote --tags origin | grep "${TAG}"`
    if [[ "$has_tag" == "" ]]; then
	export has_tag=`git ls-remote origin | grep "${TAG}"`
	if [[ "$has_tag" == "" ]]; then
	    echo "Tag or branch ${TAG} does not exist in repository."
	    exit 1
	fi
    else
	TYPE=tag
    fi
fi

#
# Build the image
#
echo "Docker will build the ${TYPE} ${TAG} from repository."
echo ""
echo "Are you sure you want to continue? (y/n)"
read input
if [[ $input == n* || $input == N* ]]; then
    exit 0
fi

docker build -t mrv2_builder .

#
# Run the compile and package extraction.  We must run it as 1001 user
# not root as creating the docs otherwise fails.
#
docker run -v ${PWD}/packages:/packages \
       --name mrv2_build_$(date "+%s") \
       mrv2_builder $TAG
