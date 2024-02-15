#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
#
# Main build script for mrv2.  It builds all dependencies and will install the
# main executable on BUILD-OS-ARCH/BUILD_TYPE/install/bin.
#
# On Linux and macOS, it will also create a mrv2 or mrv2-dbg symbolic link
# in $HOME/bin if the directory exists.
#
# This script does *NOT* save a log to  BUILD-OS-ARCH/BUILD_TYPE/compile.log.
# Use runme.sh for that.
#
#

if [[ !$RUNME ]]; then
    . $PWD/etc/build_dir.sh
fi

#
# These are some of the expensive mrv2 options
#
if [ -z "$BUILD_PYTHON" ]; then
    export BUILD_PYTHON=ON
fi

if [ -z "$MRV2_PYFLTK" ]; then
    export MRV2_PYFLTK=ON
fi

if [ -z "$MRV2_PYBIND11" ]; then
    export MRV2_PYBIND11=ON
fi

if [ -z "$MRV2_NETWORK" ]; then
    export MRV2_NETWORK=ON
fi

if [ -z "$MRV2_PDF" ]; then
    export MRV2_PDF=ON
fi

#
# These are some of the expensive TLRENDER options
#

if [ -z "$TLRENDER_ASAN" ]; then
    export TLRENDER_ASAN=OFF # asan memory debugging (not yet working)
fi

if [ -z "$TLRENDER_AV1" ]; then
    export TLRENDER_AV1=ON
fi

if [ -z "$TLRENDER_FFMPEG" ]; then
    export TLRENDER_FFMPEG=ON
fi

if [ -z "$TLRENDER_NDI_SDK" ]; then
    if [[ $KERNEL == *Linux* ]]; then
	export TLRENDER_NDI_SDK="/home/gga/code/lib/NDI SDK for Linux/"
    elif [[ $KERNEL == *Msys* ]]; then
	export TLRENDER_NDI_SDK="C:/Program Files/NDI/NDI 5 SDK/"
    else
	export TLRENDER_NDI_SDK="/Library/NDI SDK for Apple/"
    fi
fi

if [ -z "$TLRENDER_NDI" ]; then
    if [ -d "${TLRENDER_NDI_SDK}" ]; then
	export TLRENDER_NDI=ON
    else
	export TLRENDER_NDI=OFF
    fi
fi

if [ -z "$TLRENDER_NET" ]; then
    export TLRENDER_NET=ON
fi

if [ -z "$TLRENDER_RAW" ]; then
    export TLRENDER_RAW=ON
fi

if [ -z "$TLRENDER_USD" ]; then
    export TLRENDER_USD=ON
fi

if [ -z "$TLRENDER_VPX" ]; then
    export TLRENDER_VPX=ON
fi

if [ -z "$TLRENDER_WAYLAND" ]; then
    export TLRENDER_WAYLAND=ON
fi

if [ -z "$TLRENDER_YASM" ]; then
    export TLRENDER_YASM=ON
fi

echo
echo
echo "Building summary"
echo "================"
echo
echo "Build directory is ${BUILD_DIR}"
echo "Version to build is v${mrv2_VERSION}"
echo "Architecture is ${ARCH}"
echo "Building with ${COMPILER_VERSION}, ${CPU_CORES} cores"
echo "Compiler flags are ${FLAGS}"
echo "$CMAKE_VERSION"
echo


mkdir -p $BUILD_DIR/install

if [[ $KERNEL == *Linux* ]]; then
    echo "Common options"
    echo
    echo "Wayland support .................... ${TLRENDER_WAYLAND} 	(TLRENDER_WAYLAND)"
    echo
fi

echo "mrv2 Options"
echo 
echo "Build Python........................ ${BUILD_PYTHON} 	(BUILD_PYTHON)"
echo "Build pyFLTK........................ ${MRV2_PYFLTK} 	(MRV2_PYFLTK)"
echo "Build embedded Python............... ${MRV2_PYBIND11} 	(MRV2_PYBIND11)"
echo "Build mrv2 Network connections...... ${MRV2_NETWORK} 	(MRV2_NETWORK)"
echo "Build PDF........................... ${MRV2_PDF} 	(MRV2_PDF)"
echo
echo "tlRender Options"
echo

echo "FFmpeg support ..................... ${TLRENDER_FFMPEG} 	(TLRENDER_FFMPEG)"
if [[ $TLRENDER_FFMPEG == ON || $TLRENDER_FFMPEG == 1 ]]; then
    echo "FFmpeg GPL ......................... ${FFMPEG_GPL} 	(-gpl flag)"
    echo "FFmpeg network support ............. ${TLRENDER_NET} 	(TLRENDER_NET)"
    echo "AV1 codec support .................. ${TLRENDER_AV1} 	(TLRENDER_AV1)"
    echo "VPX codec support .................. ${TLRENDER_VPX} 	(TLRENDER_VPX)"
    echo "YASM assembler ..................... ${TLRENDER_YASM} 	(TLRENDER_YASM)"
fi
echo

echo "NDI support ........................ ${TLRENDER_NDI} 	(TLRENDER_NDI)"
if [[ $TLRENDER_NDI == ON || $TLRENDER_NDI == 1 ]]; then
    echo "NDI SDK ... ${TLRENDER_NDI_SDK} 	(TLRENDER_NDI_SDK}"
fi

echo
echo "LibRaw support ..................... ${TLRENDER_RAW} 	(TLRENDER_RAW)"
echo "USD support ........................ ${TLRENDER_USD} 	(TLRENDER_USD)"

sleep 10

mkdir -p $BUILD_DIR/install

#
# Handle Windows pre-flight compiles
#
if [[ $KERNEL == *Msys* ]]; then
    . $PWD/etc/compile_windows_dlls.sh
fi


cd $BUILD_DIR


cmd="cmake -G '${CMAKE_GENERATOR}' \
	   -D CMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
	   -D CMAKE_INSTALL_PREFIX=$PWD/install \
	   -D CMAKE_PREFIX_PATH=$PWD/install \
	   -D BUILD_PYTHON=${BUILD_PYTHON} \
	   -D MRV2_NETWORK=${MRV2_NETWORK} \
	   -D MRV2_PYFLTK=${MRV2_PYFLTK} \
	   -D MRV2_PYBIND11=${MRV2_PYBIND11} \
	   -D MRV2_PDF=${MRV2_PDF} \
           -D TLRENDER_ASAN=${TLRENDER_ASAN} \
           -D TLRENDER_AV1=${TLRENDER_AV1} \
           -D TLRENDER_FFMPEG=${TLRENDER_FFMPEG} \
	   -D TLRENDER_NDI=${TLRENDER_NDI} \
	   -D TLRENDER_NDI_SDK=\"${TLRENDER_NDI_SDK}\" \
	   -D TLRENDER_NET=${TLRENDER_NET} \
	   -D TLRENDER_NFD=OFF \
	   -D TLRENDER_RAW=${TLRENDER_RAW} \
	   -D TLRENDER_USD=${TLRENDER_USD} \
	   -D TLRENDER_VPX=${TLRENDER_VPX} \
	   -D TLRENDER_WAYLAND=${TLRENDER_WAYLAND} \
	   -D TLRENDER_YASM=${TLRENDER_YASM} \
	   -D TLRENDER_PROGRAMS=OFF \
	   -D TLRENDER_EXAMPLES=FALSE \
	   -D TLRENDER_TESTS=FALSE \
	   -D TLRENDER_QT6=OFF \
	   -D TLRENDER_QT5=OFF \
	   ${CMAKE_FLAGS} ../.."

run_cmd $cmd

run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE

cd -

if [[ "$CMAKE_TARGET" == "" ]]; then
    CMAKE_TARGET=install
fi

cmd="./runmeq.sh ${CMAKE_BUILD_TYPE} -t ${CMAKE_TARGET}"
run_cmd $cmd

if [[ "$CMAKE_TARGET" != "package" ]]; then
    . $PWD/etc/build_end.sh
fi

