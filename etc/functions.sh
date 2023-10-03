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
    time eval command $*
}

#
# Get kernel and architecture
#
get_kernel()
{
    export KERNEL=`uname`
    if [[ $KERNEL == *MSYS* || $KERNEL == *MINGW* ]]; then
	export KERNEL=Msys
	export ARCH=`which cl.exe`
    fi

    if [[ $ARCH == "" ]]; then
	export ARCH=`uname -m` # was uname -a
    fi

    if [[ $ARCH == arm64 ]]; then
	export ARCH=arm64
    elif [[ $ARCH == *64* ]]; then
	export ARCH=amd64
    else
	export ARCH=i386
    fi
}

#
# Extract version from cmake/version.cmake
#
extract_version()
{
    local major=`cat cmake/version.cmake | grep -o 'VERSION_MAJOR\s*[0-9]' | sed -e 's/VERSION_MAJOR[ \t]*//'`
    local minor=`cat cmake/version.cmake | grep -o 'VERSION_MINOR\s*[0-9]' | sed -e 's/VERSION_MINOR[ \t] *//'`
    local patch=`cat cmake/version.cmake | grep -o 'VERSION_PATCH\s*[0-9]-*[a-z]*[A-Z]*-*[0-9]*' | sed -e 's/VERSION_PATCH[ \t]*//'`
    export mrv2_VERSION="${major}.${minor}.${patch}"
}

#
# Extract python version from cmake/Modules/BuildPython.cmake
#
extract_python_version()
{
    local major=`cat cmake/Modules/BuildPython.cmake | grep -o 'set.[ \t]*Python_VERSION\s*[0-9]*' | sed -e 's/set.[ \t]*Python_VERSION[ \t]*//'`
    local minor=`cat cmake/Modules/BuildPython.cmake | grep -o 'set.[ \t]*Python_VERSION\s*[0-9]*\.[0-9]*' | sed -e 's/set.[ \t]*Python_VERSION[ \t]*[0-9]*\.//'`
    export PYTHON_VERSION="${major}.${minor}"
}

#
# Auxiliary function to send from the staging location to the packages/
#
send_to_packages()
{
    local stage=$PWD/$BUILD_DIR/mrv2/src/mrv2-build
    local package=$stage/$1
    if [[ "$CMAKE_TARGET" != "" ]]; then
	mkdir -p $PWD/packages
	if [[ -e $package ]]; then
	    echo "Installing $package in $PWD/packages"
	    run_cmd mv $package $PWD/packages
	else
	    echo "ERROR package $1 was not created in $stage."
	fi
    else
	echo "CMAKE_TARGET is empty.  Will not copy packages."
    fi
}
