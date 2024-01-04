#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



show_help()
{
    if [[ $RUNME == 1 ]]; then
	echo "$0 [debug] [clean] [dist] [-v] [-j <num>] [-lgpl] [-gpl] [-D VAR=VALUE] [-t <target>] [-help]"
	echo ""
	echo "* debug builds a debug build."
	echo "* clean clears the directory before building -- use only with runme.sh"
	echo "* dist builds a Mojave compatible distribution (macOS)."
	echo "* -j <num>  controls the threads to use when compiling. [default=$CPU_CORES]"
	echo "* -v builds verbosely. [default=off]"
	echo "* -D sets cmake variables, like -D TLRENDER_USD=OFF."
	echo "* -gpl builds FFmpeg with x264 encoder support in a GPL version of it."
	echo "* -lgpl builds FFmpeg as a LGPL version of it."
	echo "* -t <target> sets the cmake target to run. [default=none]"
    else
	echo "$0 [debug] [-j <num>] [-help]"
	echo ""
	echo "* debug builds a debug build."
	echo "* -j <num>  controls the threads to use when compiling. [default=$CPU_CORES]"
    fi
}

parse_option()
{
    local input="$1"
    # Use regular expressions to match the option and value
    if [[ "$input" =~ ^(-D)?(.+)=(.+)$ ]]; then
        local option="${BASH_REMATCH[2]}"
        local value="${BASH_REMATCH[3]}"
	export CMAKE_FLAGS="-D $option=$value ${CMAKE_FLAGS}"
    else
        echo "Invalid option format: $input"
        exit 1
    fi
}

#
# Get the KERNEL and ARCH variables
#
get_kernel

export RUNME=0
if [[ $0 == *runme.sh* || $0 == *runme_minimal.sh* ]]; then
    RUNME=1
fi

BUILD_ROOT=BUILD-$KERNEL-$ARCH


echo "COMMAND=$0"

export MRV2_DIST_RELEASE=0
export FFMPEG_GPL=$FFMPEG_GPL
CLEAN_DIR=0
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
	    export MRV2_DIST_RELEASE=1
	    shift
	    ;;
	--build-dir|-build-dir|--dir|-dir|--root|-root)
	    shift
	    BUILD_ROOT=$1
	    shift
	    ;;
	clean)
	    if [[ $RUNME == 0 ]]; then
		echo "COMMNAD=$0"
		echo "WARNING: clean option can only be run with the runme.sh script"
	    else
		CLEAN_DIR=1
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
	-dir|--dir|-build-dir|--build-dir|-root|--root-dir)
	    shift
	    BUILD_ROOT=$1
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
	    export FLAGS="-j $CPU_CORES ${FLAGS}"
	    shift
	    ;;
	-D*)
	    if [[ $1 == "-D" ]]; then
	       shift
	    fi
	    parse_option $1
	    shift
	    ;;
	-G)
	    shift
	    if [[ $RUNME == 0 ]]; then
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
	    show_help
	    exit 1
	    ;;
    esac
done

