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

export CLEAN_DIR=0
export CMAKE_OSX_ARCHITECTURES=""
export SHOW_INCLUDES=0
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
	    export FLAGS="-v ${FLAGS}"
	    shift
	    ;;
	-j)
	    shift
	    export CPU_CORES=$1
	    shift
	    ;;
	-system-python|--system-python)
	    shift
	    export CMAKE_FLAGS="-D BUILD_PYTHON=OFF ${CMAKE_FLAGS}"
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
	-t)
	    shift
	    export CMAKE_TARGET=$1
	    shift
	    ;;
	-h*)
	    echo "$0 [debug] [clean] [dist] [-v] [-j <num>] [-system-python] [-help]"
	    echo ""
	    echo "* debug builds a debug build."
	    echo "* clean clears the directory before building -- use only with runme.sh"
	    echo "* dist builds a Mojave compatible distribution (macOS)."
	    echo "* -j <num>  controls the threads to use when compiling."
	    echo "* -v builds verbosely."
	    echo "* -system-python Use system's python instead of compiling it."
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

export PATH="$PWD/${BUILD_DIR}/install/bin:$PWD/$BUILD_DIR/install/bin/Scripts:${PATH}"
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

echo "Build directory is ${BUILD_DIR}"

if [[ $CLEAN_DIR == 1 ]]; then
    if [[ -d ${BUILD_DIR} ]]; then
	echo "Cleaning ${BUILD_DIR}.  Please wait..."
	rm -rf $BUILD_DIR
    fi
fi

echo "Version to build is v${mrv2_VERSION}"
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
