#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Some auxiliary functions
#

#
# Simple function to run a command and print it out
#
run_cmd()
{
    echo
    echo "> $*"
    echo
    eval command $*
}


#
# Extract version from cmake/version.cmake
#
extract_version()
{
    local major=`cat cmake/version.cmake | grep -Po '(VERSION_MAJOR\s)\d' | sed -e 's/VERSION_MAJOR\s*//'`
    local minor=`cat cmake/version.cmake | grep -Po '(VERSION_MINOR\s)\d' | sed -e 's/VERSION_MINOR\s*//'`
    local patch=`cat cmake/version.cmake | grep -Po '(VERSION_PATCH\s)\d' | sed -e 's/VERSION_PATCH\s*//'`
    export mrv2_VERSION="${major}.${minor}.${patch}"
}

#
# Auxiliary function to send from the staging location to the packages/
#
send_to_packages()
{
    local package=$PWD/$BUILD_DIR/mrv2/src/mrv2-build/$1
    mkdir -p $PWD/packages
    if [[ -e $package ]]; then
	echo "Installing $package in $PWD/packages"
	mv $package $PWD/packages
    fi
}
