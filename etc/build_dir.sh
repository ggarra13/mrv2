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

extract_version

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

if [[ $ARCH == arm64 ]]; then
    export ARCH=arm64
elif [[ $ARCH == *64* ]]; then
    export ARCH=amd64
else
    export ARCH=i386
fi

export DIST=0

. etc/build_cores.sh

export FFMPEG_GPL=$FFMPEG_GPL
export CLEAN_DIR=0
export CMAKE_OSX_ARCHITECTURES=""
export CMAKE_BUILD_TYPE="Release"
export CMAKE_GENERATOR="Ninja"
export CMAKE_TARGET=""
for i in $@; do
    case $i in
	release|Release)
	    export CMAKE_BUILD_TYPE="Release"
	    shift
	    ;;
	debug|Debug)		
	    export CMAKE_BUILD_TYPE="Debug"
	    export CMAKE_FLAGS=" -DTLRENDER_API=GL_4_1_Debug"
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
	-lgpl|--lgpl)
	    export FFMPEG_GPL=LGPL
	    export CMAKE_FLAGS="-D TLRENDER_X264=OFF ${CMAKE_FLAGS}"
	    shift
	    ;;
	-gpl|--gpl)
	    export FFMPEG_GPL=GPL
	    export CMAKE_FLAGS="-D TLRENDER_X264=ON ${CMAKE_FLAGS}"
	    shift
	    ;;
	-v)
	    export CMAKE_FLAGS="-D CMAKE_VERBOSE_MAKEFILE=ON ${CMAKE_FLAGS}"
	    export FLAGS="-v ${FLAGS}"
	    shift
	    ;;
	-j)
	    shift
	    export CPU_CORES=$1
	    shift
	    ;;
	-D)
	    shift
	    export CMAKE_FLAGS="-D $1 ${CMAKE_FLAGS}"
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
	-t|--t|--target)
	    shift
	    export CMAKE_TARGET=$1
	    shift
	    ;;
	-h|-help|--help)
	    echo "$0 [debug] [clean] [dist] [-v] [-j <num>] [-lgpl] [-gpl] [-D VAR=VALUE] [-help]"
	    echo ""
	    echo "* debug builds a debug build."
	    echo "* clean clears the directory before building -- use only with runme.sh"
	    echo "* dist builds a Mojave compatible distribution (macOS)."
	    echo "* -j <num>  controls the threads to use when compiling."
	    echo "* -v builds verbosely."
	    echo "* -D sets cmake variables, like -D TLRENDER_USD=OFF."
	    echo "* -gpl builds FFmpeg with x264 encoder support in a GPL version of it."
	    echo "* -lgpl builds FFmpeg as a LGPL version of it."
	    exit 1
	    ;;
    esac
done



# Build a build directory with that information
export BUILD_DIR=BUILD-$KERNEL-$ARCH/$CMAKE_BUILD_TYPE

export PATH="$PWD/${BUILD_DIR}/install/bin:$PWD/$BUILD_DIR/install/bin/Scripts:${PATH}"
export PKG_CONFIG_PATH="$PWD/${BUILD_DIR}/install/lib/pkgconfig:$PKG_CONFIG_PATH"
if [[ $KERNEL == *Darwin* ]]; then
    export PATH="/usr/local/opt/gnu-sed/libexec/gnubin:${PATH}"
    if [[ $ARCH == arm64 ]]; then
	export CMAKE_OSX_ARCHITECTURES=$ARCH
    fi
    if [[ $DIST == 1 ]]; then
	export CMAKE_FLAGS="-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 ${CMAKE_FLAGS}"
    fi
fi

export FLAGS="${FLAGS} $*"
export FLAGS="-j ${CPU_CORES} ${FLAGS}"

if [[ $CLEAN_DIR == 1 ]]; then
    if [[ -d ${BUILD_DIR} ]]; then
	echo "Cleaning ${BUILD_DIR}.  Please wait..."
	rm -rf $BUILD_DIR
    fi
fi

if [[ $0 == *runme.sh* ]]; then
    echo "Build directory is ${BUILD_DIR}"
    echo "Version to build is v${mrv2_VERSION}"
    echo "Architecture is ${ARCH}"
    echo "FFmpeg will be built as ${FFMPEG_GPL}"
    echo "CMake flags are ${CMAKE_FLAGS}"
    echo "Compiler flags are ${FLAGS}"
    cmake --version
    mkdir -p $BUILD_DIR/install

    if [[ $FFMPEG_GPL == LGPL ]]; then
	rm -rf $BUILD_DIR/install/bin/libx264*.dll
	rm -rf $BUILD_DIR/install/lib/libx264.lib
    fi
fi

if [[ $0 == *runme.sh* ]]; then
    if [[ $KERNEL == *Msys* ]]; then
	. $PWD/etc/compile_windows_dlls.sh
    fi
fi
