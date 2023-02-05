#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



#
# Turn on exit on error
#
set -o pipefail -e

#
# Get the auxiliary functions
#
. etc/functions.sh

#
# Determine OS Kernel, OS CPU architecture
#
export KERNEL=`uname`
if [[ $KERNEL == *MSYS* || $KERNEL == *MINGW* ]]; then
    export KERNEL=Msys
    export ARCH=`which cl.exe`
fi

if [[ $ARCH == "" ]]; then
    export ARCH=`uname -m` # was uname -a
fi


if [[ $ARCH == *64* ]]; then
    ARCH=amd64
else
    ARCH=i386
fi

export DIST=0
export CPU_CORES=4
export CLEAN_DIR=0
export CMAKE_OSX_ARCHITECTURES=""
export SHOW_INCLUDES=0
export CMAKE_BUILD_TYPE="Release"
export CMAKE_GENERATOR="Ninja"
for i in $@; do
    case $i in
	debug)
	    export CMAKE_BUILD_TYPE="Debug"
	    shift
	    ;;
	dist)
	    export DIST=1
	    if [[ $0 != *runme.sh* ]]; then
		echo $0
		echo "dist option can only be run with the runme.sh script"
		exit 1
	    fi
	    shift
	    ;;
	clean)
	    export CLEAN_DIR=1
	    if [[ $0 != *runme.sh* ]]; then
		echo $0
		echo "clean option can only be run with the runme.sh script"
		exit 1
	    fi
	    shift
	    ;;
	-v)
	    export CMAKE_FLAGS="-D CMAKE_VERBOSE_MAKEFILE=ON ${CMAKE_FLAGS}"
	    shift
	    ;;
	-j)
	    shift
	    export CPU_CORES=$1
	    shift
	    ;;
	-G)
	    shift
	    if [[ $0 != *runme.sh* ]]; then
		echo $0
		echo "Cmake generator can only be run with the runme.sh script"
		exit 1
	    fi
	    export CMAKE_GENERATOR=$1
	    shift
	    ;;
	-h*)
	    echo "$0 [debug] [clean] [dist] -v -j <num> [-help]"
	    echo ""
	    echo "* debug builds a debug build."
	    echo "* clean clears the directory before building -- use only with runme.sh"
	    echo "* dist builds a Mojave compatible distribution (macOS)"
	    echo "* -j <num>  controls the threads to use when compiling"
	    echo "* -v builds verbosely"
	    exit 1
	    ;;
    esac
done



# Build a build directory with that information
export BUILD_DIR=BUILD-$KERNEL-$ARCH/$CMAKE_BUILD_TYPE

if [[ $TLRENDER_QT6 == "ON" ]]; then
    export BUILD_DIR=Qt6/$BUILD_DIR
elif [[ $TLRENDER_QT5 == "ON" ]]; then
    export BUILD_DIR=Qt5/$BUILD_DIR
fi

if [[ $KERNEL == *Darwin* ]]; then
    export PATH="/usr/local/opt/gnu-sed/libexec/gnubin:$PATH"
    if [[ $DIST == 1 ]]; then
	export CMAKE_FLAGS="-DCMAKE_OSX_DEPLOYMENT_TARGET=10.14 ${CMAKE_FLAGS}"
    fi
fi

export FLAGS="-v $*"
export FLAGS="-j ${CPU_CORES} ${FLAGS}"

echo "Build directory is ${BUILD_DIR}"

if [[ $CLEAN_DIR == 1 ]]; then
    if [[ -d ${BUILD_DIR} ]]; then
	echo "Cleaning ${BUILD_DIR}.  Please wait..."
	rm -rf $BUILD_DIR
    fi
fi

echo "Architecture is ${ARCH}"
echo "CMake flags are ${CMAKE_FLAGS}"
echo "Compiler flags are ${FLAGS}"

if [[ $KERNEL == *Msys* ]]; then
    . $PWD/etc/windows_prepare.sh
    echo "FFMPEG_ROOT=${FFMPEG_ROOT}"
fi

if [[ $0 == *runme.sh* ]]; then
    #
    # First run, create the standard directories.
    #
    mkdir -p $BUILD_DIR/install/bin $BUILD_DIR/install/lib
    mkdir -p $BUILD_DIR/install/include

    if [[ $KERNEL == *Msys* ]]; then
	. $PWD/etc/copy_dlls.sh
    fi
fi
