#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



show_help()
{
    if [[ $RUNME == 1 ]]; then
	echo "$0 [debug|release|reldeb] [clean] [-v] [-j <num>] [-lgpl] [-gpl] [-D VAR=VALUE] [-t <target>] [-help]"
	echo ""
	echo "* debug builds a debug build."
	echo "* release builds a release build. (default)"
	echo "* reldeb  builds a release build with debugging symbols."
	echo "* clean clears the directory before building -- use only with runme.sh"
	echo "* -j <num>  controls the threads to use when compiling. [default=$CPU_CORES]"
	echo "* -v builds verbosely. [default=off]"
	echo "* -D sets cmake variables, like -D TLRENDER_USD=OFF."
	echo "* -gpl builds FFmpeg with x264 encoder support in a GPL version of it."
	echo "* -lgpl builds FFmpeg as a LGPL version of it."
	echo "* -t <target> sets the cmake target to run. [default=none]"
    else
	echo "$0 [debug|release|reldeb] [-j <num>] [-help]"
	echo ""
	echo "* debug builds a debug build."
	echo "* release builds a release build. (default)"
	echo "* reldeb  builds a release build with debugging symbols."
	echo "* -j <num>  controls the threads to use when compiling. [default=$CPU_CORES]"
    fi
}

parse_option()
{
    local input="$@"
    # Use regular expressions to match the option and value
    if [[ "$input" =~ ^(-D)?(.+)=(.+)$ ]]; then
        local option="${BASH_REMATCH[2]}"
        local value="${BASH_REMATCH[3]}"
	export "$option=$value"
    else
        echo "Invalid option format: $input"
        exit 1
    fi
}

#
# Set RUNME variable
#
export RUNME=0
if [[ $0 == *runme*.sh ]]; then
    if [[ $0 == *runmet.sh* || $0 == *runmeq.sh* ||
	      $0 == *runme_nolog.sh* ]]; then
	RUNME=0
    else
	RUNME=1
    fi
fi


#
# Get the KERNEL and ARCH variables
#
get_kernel

#
# With KERNEL and ARCH, build default root dir
#
default_build_root=BUILD-$KERNEL-$ARCH

#
# Set up parse variables' default values
#
CLEAN_DIR=0

if [[ -z $FFMPEG_GPL ]]; then
    export FFMPEG_GPL=LGPL
fi

if [[ -z $TLRENDER_X264 ]]; then
    export TLRENDER_X264=OFF
fi

if [[ -z $CMAKE_OSX_ARCHITECTURES ]]; then
    export CMAKE_OSX_ARCHITECTURES="x86_64"
    if [[ $ARCH == *arm64* ]]; then
	export CMAKE_OSX_ARCHITECTURES="arm64"
    fi
fi
if [[ -z $CMAKE_VERBOSE_MAKEFILE ]]; then
    export CMAKE_VERBOSE_MAKEFILE=OFF
fi
if [[ -z $CMAKE_BUILD_TYPE ]]; then
   export CMAKE_BUILD_TYPE="Release"
fi
if [[ -z $CMAKE_GENERATOR ]]; then
    export CMAKE_GENERATOR="Ninja"
fi
if [[ -z $CMAKE_TARGET ]]; then
    export CMAKE_TARGET=""
fi
if [[ -z $ASK_TO_CONTINUE ]]; then
    export ASK_TO_CONTINUE=0
fi
if [[ -z $INSTALL_CMAKE ]]; then
    if [[ "$USER" == "User-PC" ||
	      "$USER" == "gga" ]]; then
	export INSTALL_CMAKE=1
    else
	export INSTALL_CMAKE=1
    fi
fi

if [[ "$NOARGS" == "" ]]; then
    
    while [ $# -gt 0 ]; do
	case $1 in
	    reldeb|RelWithDebInfo)
		export CMAKE_BUILD_TYPE="RelWithDebInfo"
		shift
		continue
		;;
	    release|Release)
		export CMAKE_BUILD_TYPE="Release"
		shift
		continue
		;;
	    debug|Debug)		
		export CMAKE_BUILD_TYPE="Debug"
		shift
		continue
		;;
	    -y|-yes|--y|--yes)
		shift
		ASK_TO_CONTINUE=0
		continue
		;;
	    --ask|-ask)
		shift
		ASK_TO_CONTINUE=1
		continue
		;;
	    --build-dir|-build-dir|--dir|-dir|--root|-root)
		shift
		export BUILD_ROOT=$1
		echo "BUILD_ROOT set to $BUILD_ROOT"
		shift
		continue
		;;
	    --small|-small)
		shift
		export BUILD_ROOT=$default_build_root-small
		continue
		;;
	    --minimal|-minimal|--min|-min)
		shift
		export BUILD_ROOT=$default_build_root-minimal
		continue
		;;
	    --no-local-cmake|-no-cmake|--no-cmake)
		shift
		export INSTALL_CMAKE=0
		continue
		;;
	    clean)
		CLEAN_DIR=1
		if [[ $RUNME == 0 && $RUNME_NOLOG == 0 ]]; then
		    echo $0
		    echo "clean option can only be run with the runme.sh script"
		    exit 1
		fi
		shift
		continue
		;;
	    -lgpl|--lgpl)
		export FFMPEG_GPL=LGPL
		export TLRENDER_X264=OFF
		shift
		continue
		;;
	    -gpl|--gpl)
		export FFMPEG_GPL=GPL
		export TLRENDER_X264=ON
		shift
		continue
		;;
	    -v|--v|--verbose)
		export CMAKE_VERBOSE_MAKEFILE=ON
		export FLAGS="-v ${FLAGS}"
		shift
		continue
		;;
	    -vk|--vk|--vulkan)
		export MRV2_HDR=ON
		export MRV2_VK=ON
		export MRV2_BACKEND=VK
		export TLRENDER_VK=ON
		shift
		continue
		;;
	    -j)
		shift
		export CPU_CORES=$1
		export FLAGS="-j $CPU_CORES ${FLAGS}"
		shift
		continue
		;;
	    -D*)
		if [[ $1 == "-D" ]]; then
		    shift
		fi
		parse_option $1
		shift
		continue
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
		continue
		;;
	    -t|--t|--target)
		shift
		export CMAKE_TARGET=$1
		shift
		continue
		;;
	    -h|-help|--help)
		show_help
		exit 1
		;;
	    *)
		echo "Unknown option: $1"
		show_help
		exit 1
	esac
    done

fi

if [[ "$MRV2_BACKEND" == "VK" && "$BUILD_ROOT" == "" ]]; then
    export BUILD_ROOT=${KERNEL}-vulkan-${ARCH}
fi
