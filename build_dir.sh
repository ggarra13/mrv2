#!/bin/bash

#
# Determine OS Kernel, OS Release and CPU architecture
#
export KERNEL=`uname -s`
export RELEASE=`uname -r`
if [[ $KERNEL == *MSYS* ]]; then
    export ARCH=`which cl.exe`
fi

if [[ $ARCH == "" ]]; then
    export ARCH=`uname -a`
fi


if [[ $ARCH == *64* ]]; then
    ARCH=64
else
    ARCH=32
fi

export CLEAN_DIR=0
export CMAKE_BUILD_TYPE="Release"
for i in $@; do
    case $i in
        debug)
            export CMAKE_BUILD_TYPE="Debug"
	    shift
            break
            ;;
	clean)
	    export CLEAN_DIR=1
	    shift
            break
	    ;;
	-h*)
	    echo "$0 [debug] [clean] [-help]"
	    exit 1
	    ;;
    esac
done

# Build a build directory with that information
export LIBINTL_ROOT=/E/code/lib/win64/libintl-win64
export BUILD_DIR=BUILD-$KERNEL-$RELEASE-$ARCH/$CMAKE_BUILD_TYPE
export FLAGS=$@
if [[ $FLAGS == "" ]]; then
    export FLAGS="-j 4"
fi

echo "Build directory is ${BUILD_DIR}"

if [[ $CLEAN_DIR == 1 ]]; then
    if [[ -d ${BUILD_DIR} ]]; then
	echo "Cleaning ${BUILD_DIR}"
	rm -rf $BUILD_DIR
    fi
fi


echo "Flags are ${FLAGS}"
echo "LIBINTL_ROOT is ${LIBINTL_ROOT}"

mkdir -p $BUILD_DIR/install/bin $BUILD_DIR/install/lib $BUILD_DIR/install/include

if [[ $KERNEL == *MSYS* ]]; then
    . copy_ffmpeg.sh
fi
