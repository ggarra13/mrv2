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
#
# Install cmake
#
if [[ !$RUNME ]]; then
    . $PWD/etc/build_dir.sh
fi

#
# Set environment variables to point to install directory
#
export PATH="$PWD/${BUILD_DIR}/install/bin:$PWD/$BUILD_DIR/install/bin/Scripts:${PATH}"
echo "PATH is now set now to ${PATH}"
echo "It has:"
mkdir -p $PWD/${BUILD_DIR}/install/bin/


#
# Extract cmake version
#
. etc/install_cmake.sh

get_cmake_version


#
# These are some of the expensive mrv2 options
#
if [ -z "$BUILD_GETTEXT" ]; then
    export BUILD_GETTEXT=OFF
fi

if [ -z "$BUILD_PYTHON" ]; then
    export BUILD_PYTHON=ON
fi

if [ -z "$BUILD_WAYLAND" ]; then
    export BUILD_WAYLAND=ON
fi

if [ -z "$BUILD_X11" ]; then
    export BUILD_X11=ON
fi

if [ -z "$MRV2_BACKEND" ]; then
    export MRV2_BACKEND=GL
fi

if [ -z "$MRV2_CPPTRACE" ]; then
    export MRV2_CPPTRACE=ON
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


if [ -z "$MRV2_PYTHON" ]; then
    if [[ $BUILD_PYTHON == ON || $BUILD_PYTHON == 1 ]]; then
	if [[ $KERNEL == *Windows* ]]; then
	    export PYTHONEXE=python.exe
	else
	    export PYTHONEXE=python3
	fi
	export PYTHON="${PWD}/${BUILD_DIR}/install/bin/${PYTHONEXE}"
    else
	locate_python
    fi
    export MRV2_PYTHON=$PYTHON
    export TLRENDER_USD_PYTHON=$PYTHON
else
    export TLRENDER_USD_PYTHON=$MRV2_PYTHON
    export PYTHON=$MRV2_PYTHON
fi

#
# These are some of the expensive TLRENDER options
#

if [[ -z "$TLRENDER_API" ]]; then
    export TLRENDER_API=GL_4_1
    if [[ "$CMAKE_BUILD_TYPE" == "Debug" ]]; then
	export TLRENDER_API=GL_4_1_Debug
    fi
fi

if [ -z "$TLRENDER_ASAN" ]; then
    export TLRENDER_ASAN=OFF # asan memory debugging (not yet working)
fi

if [ -z "$TLRENDER_AOM" ]; then
    if [[ $ARCH == *aarch64* ]]; then
	export TLRENDER_AOM=ON
    else
	export TLRENDER_AOM=OFF
    fi
fi

if [ -z "$TLRENDER_AV1" ]; then
    export TLRENDER_AV1=ON
fi

if [ -z "$TLRENDER_EXR" ]; then
    export TLRENDER_EXR=ON
fi

if [ -z "$TLRENDER_GL" ]; then
    export TLRENDER_GL=ON
fi

if [ -z "$TLRENDER_HAP" ]; then
    export TLRENDER_HAP=ON
fi

if [ -z "$TLRENDER_JPEG" ]; then
    export TLRENDER_JPEG=ON
fi

if [ -z "$TLRENDER_FFMPEG" ]; then
    export TLRENDER_FFMPEG=ON
fi

if [ -z "$TLRENDER_FFMPEG_MINIMAL" ]; then
    export TLRENDER_FFMPEG_MINIMAL=ON
fi

if [ -z "$TLRENDER_LIBPLACEBO" ]; then
    if [[ $TLRENDER_FFMPEG == ON || $TLRENDER_FFMPEG == 1 ]]; then
	export TLRENDER_LIBPLACEBO=ON
    else
	export TLRENDER_LIBPLACEBO=OFF
    fi
fi

if [ -z "$TLRENDER_LOCAL" ]; then
    export TLRENDER_LOCAL=OFF
fi

if [ -z "$TLRENDER_NDI_SDK" ]; then
    if [[ $KERNEL == *Linux* ]]; then
	export TLRENDER_NDI_SDK="$HOME/code/lib/NDI Advanced SDK for Linux/"
    elif [[ $KERNEL == *Windows* ]]; then
	export TLRENDER_NDI_SDK="C:/Program Files/NDI/NDI 6 Advanced SDK/"
    else
	export TLRENDER_NDI_SDK="/Library/NDI Advanced SDK for Apple/"
    fi
fi

export MRV2_HDR=OFF
if [ -z "$TLRENDER_NDI" ]; then
    if [ -d "${TLRENDER_NDI_SDK}" ]; then
	export TLRENDER_NDI=ON
	export MRV2_HDR=ON
    else
	echo "TLRENDER_NDI_SDK not found at ${TLRENDER_NDI_SDK}!"
	export TLRENDER_NDI=OFF
	export MRV2_HDR=OFF
    fi
fi

if [ -z "$TLRENDER_NET" ]; then
    export TLRENDER_NET=ON
fi

if [ -z "$TLRENDER_OPENJPH" ]; then
    export TLRENDER_OPENJPH=ON
fi

if [ -z "$TLRENDER_RAW" ]; then
    export TLRENDER_RAW=ON
fi

if [ -z "$TLRENDER_STB" ]; then
    export TLRENDER_STB=ON
fi

if [ -z "$TLRENDER_SVTAV1" ]; then
    export TLRENDER_SVTAV1=ON
fi

if [ -z "$TLRENDER_TIFF" ]; then
    export TLRENDER_TIFF=ON
fi

if [ -z "$TLRENDER_USD" ]; then
    export TLRENDER_USD=ON
fi

if [ -z "$VULKAN_SDK" ]; then
    export VULKAN_SDK=/crapola_of_dir
    if [[ $KERNEL == *Windows* ]]; then
	export VULKAN_SDK=/C/VulkanSDK
    elif [[ $KERNEL == *Linux* ]]; then
	if [[ -d VulkanSDK-Linux ]]; then
	    vulkan_root=$PWD/VulkanSDK-Linux
	    SDK_VERSION=$(ls -d ${vulkan_root}/* | sort -r | grep -o "$vulkan_root/[0-9]*\..*"| sed -e "s#$vulkan_root/##" | head -1)
	    export VULKAN_SDK=$vulkan_root/$SDK_VERSION/$UNAME_ARCH
	else
	    export VULKAN_SDK=/usr/
	fi
    elif [[ $KERNEL == *Darwin* ]]; then
	vulkan_root=$HOME/VulkanSDK
	if [ -d "$vulkan_root" ]; then
	    SDK_VERSION=$(ls -d ${vulkan_root}/* | sort -r | grep -o "$vulkan_root/[0-9]*\..*"| sed -e "s#$vulkan_root/##" | head -1)
	    export VULKAN_SDK=$vulkan_root/$SDK_VERSION/macOS
	else
	    if [[ -d /usr/local/include/vulkan ]]; then
		export VULKAN_SDK=/usr/local/
	    else
		export VULKAN_SDK=/opt/homebrew/
	    fi
	fi
    fi
else
    echo "Using VULKAN_SDK from environment: ${VULKAN_SDK}"
fi
    
if [ -z "$TLRENDER_VK" ]; then
    if [ -e "${VULKAN_SDK}/include/vulkan/vulkan.h" ]; then
	export TLRENDER_VK=ON
	echo "Vulkan FOUND at ${VULKAN_SDK}/include/vulkan"
    else
	export TLRENDER_VK=OFF
	export MRV2_HDR=OFF
	echo "Vulkan NOT FOUND at ${VULKAN_SDK}/include/vulkan"
    fi
else
    if [ ! -e "${VULKAN_SDK}/include/vulkan/vulkan.h" ]; then
	echo "Vulkan NOT FOUND at ${VULKAN_SDK}/include/vulkan"
	export TLRENDER_VK=OFF
	export MRV2_HDR=OFF
    else
	if [[ "$TLRENDER_VK" == "ON" || "$TLRENDER_VK" == "1" ]]; then
	    echo "Vulkan FOUND at ${VULKAN_SDK}/include/vulkan"
	else
	    export MRV2_HDR=OFF
	fi
    fi
fi

    
if [ -z "$TLRENDER_VPX" ]; then
    export TLRENDER_VPX=ON
fi

if [ -z "$TLRENDER_X264" ]; then
    export TLRENDER_X264=OFF
fi

if [ -z "$TLRENDER_WAYLAND" ]; then
    export TLRENDER_WAYLAND=${BUILD_WAYLAND}
fi

if [ -z "$TLRENDER_X11" ]; then
    export TLRENDER_X11=${BUILD_X11}
fi

if [ -z "$TLRENDER_YASM" ]; then
    export TLRENDER_YASM=ON
fi

if [ -z "$FLTK_BUILD_SHARED" ]; then
    export FLTK_BUILD_SHARED=${MRV2_PYFLTK}
    if [ -z "$FLTK_BUILD_SHARED" ]; then
	export FLTK_BUILD_SHARED=OFF
    fi
fi
    
#
# Clean python path to avoid clashes, mainly, with macOS meson
#
unset PYTHONPATH

#
# For Windows mainly, make sure we use UTF8 encoding.
#
#  export PYTHONUTF8=1  USD needs it, meson fails.

echo "Z"

echo
echo
echo "Building summary"
echo "================"
echo
echo "mrv2 version to build is v${mrv2_VERSION}"
echo
echo "Build directory is ${BUILD_DIR}"
echo "Build type      is ${CMAKE_BUILD_TYPE}"
echo "Build cores     is ${CPU_CORES}"
echo
echo "C COMPILERS"
echo "-----------"
echo "Native C compiler ${NATIVE_C_COMPILER_NAME} version ${NATIVE_C_COMPILER_VERSION}"
echo "Generic C compiler ${GENERIC_C_COMPILER_NAME} version ${GENERIC_C_COMPILER_VERSION}"
echo "Generic GNU C compiler ${GNU_C_COMPILER_NAME} ${GNU_C_COMPILER_VERSION}"
echo
echo "CXX COMPILERS"
echo "-------------"
echo "Native CXX compiler ${NATIVE_CXX_COMPILER_NAME} version ${NATIVE_CXX_COMPILER_VERSION}"
echo "Generic CXX with ${GENERIC_CXX_COMPILER_NAME} version ${GENERIC_CXX_COMPILER_VERSION}"
echo "Generic GNU CXX compiler ${GNU_CXX_COMPILER_NAME} ${GNU_CXX_COMPILER_VERSION}"
echo
echo "CMake at: ${CMAKE_LOCATION} ${CMAKE_VERSION}"
if [[ $KERNEL == *Darwin* ]]; then
    echo "Building on MacOS Brand ${MACOS_BRAND}"
fi
echo "Compiler flags are ${FLAGS}"
if [[ $KERNEL == *Windows* ]]; then
    if command -v makensis.exe > /dev/null 2>&1; then
	nsis_exe=makensis.exe
    else
	nsis_exe="/C/Program Files (x86)/NSIS/bin/makensis"
    fi
    nsis_version=`"${nsis_exe}" -version`
    echo "NSIS ${nsis_version}"
fi


if [[ $KERNEL == *Linux* ]]; then
    echo "Common options"
    echo
    echo "Wayland support .................... ${TLRENDER_WAYLAND} 	(BUILD_WAYLAND)"
    echo "X11 support ........................ ${TLRENDER_X11}     	(BUILD_X11)"
    echo
fi

echo "mrv2 Options"
echo 
echo "Build Python........................ ${BUILD_PYTHON} 	(BUILD_PYTHON)"
if [[ ${BUILD_PYTHON} == OFF || ${BUILD_PYTHON} == 0 ]]; then
    echo "Python location: ${MRV2_PYTHON}"
else
    echo "Python FUTURE location: ${MRV2_PYTHON}"
fi
echo "Build pyFLTK........................ ${MRV2_PYFLTK} 	(MRV2_PYFLTK)"
echo "Build FLTK shared................... ${FLTK_BUILD_SHARED} 	(FLTK_BUILD_SHARED)"
echo "Build embedded Python............... ${MRV2_PYBIND11} 	(MRV2_PYBIND11)"
echo "Build mrv2 Network connections...... ${MRV2_NETWORK} 	(MRV2_NETWORK)"
echo "Build PDF........................... ${MRV2_PDF} 	(MRV2_PDF)"
echo "Build hdr application............... ${MRV2_HDR} 	(MRV2_HDR)"
echo "mrv2 BACKEND........................ ${MRV2_BACKEND} 	(MRV2_BACKEND)"
echo
echo "tlRender Options"
echo

echo "FFmpeg support ..................... ${TLRENDER_FFMPEG} 	(TLRENDER_FFMPEG)"
if [[ $TLRENDER_FFMPEG == ON || $TLRENDER_FFMPEG == 1 ]]; then
    echo "FFmpeg License ..................... ${FFMPEG_GPL} 	(Use -gpl flag)"
    echo "    FFmpeg minimal.................. ${TLRENDER_FFMPEG_MINIMAL}         (TLRENDER_FFMPEG_MINIMAL)"
    echo "    FFmpeg network support ......... ${TLRENDER_NET} 	(TLRENDER_NET)"
    echo "    dav1d decodec support .......... ${TLRENDER_AV1} 	(TLRENDER_AV1)"
    if [[ $TLRENDER_SVTAV1 == ON || $TLRENDER_SVTAV1 == 1 ]]; then
	echo "    SvtAv1 codec support. .......... ${TLRENDER_SVTAV1} 	(TLRENDER_SVTAV1)"
    fi
    if [[ $TLRENDER_AOM == ON || $TLRENDER_AOM == 1 ]]; then
	echo "    AOM encodeder support. .......... ${TLRENDER_AOM} 	(TLRENDER_AOM)"
    fi
    echo "    HAP/Snappy codec support ....... ${TLRENDER_HAP} 	(TLRENDER_HAP)"
    echo "    VPX codec support .............. ${TLRENDER_VPX} 	(TLRENDER_VPX)"
    echo "    X264 codec support ............. ${TLRENDER_X264} 	(Use -gpl flag)"
    echo "    libplacebo support ............. ${TLRENDER_LIBPLACEBO}         (TLRENDER_LIBPLACEBO)"
    echo "    YASM assembler ................. ${TLRENDER_YASM} 	(TLRENDER_YASM)"
fi
echo

echo "NDI support ........................ ${TLRENDER_NDI} 	(TLRENDER_NDI)"
if [[ $TLRENDER_NDI == ON || $TLRENDER_NDI == 1 ]]; then
    echo "NDI SDK ${TLRENDER_NDI_SDK} 	(TLRENDER_NDI_SDK}"
fi
if [[ $TLRENDER_VK == ON || $TLRENDER_VK == 1 ]]; then
    echo "VULKAN_SDK    .................. ${VULKAN_SDK} 	(env. variable)"
fi
    
echo
echo "JPEG   support ..................... ${TLRENDER_JPEG} 	(TLRENDER_JPEG)"
echo "LibRaw support ..................... ${TLRENDER_RAW} 	(TLRENDER_RAW)"
echo "OpenEXR support .................... ${TLRENDER_EXR} 	(TLRENDER_EXR)"
echo "STB support (TGA, BMP, PSD) ........ ${TLRENDER_STB} 	(TLRENDER_STB)"
echo "TIFF support ....................... ${TLRENDER_TIFF} 	(TLRENDER_TIFF)"
echo "tlRender API ....................... ${TLRENDER_API} 	(TLRENDER_API)"
echo "USD support ........................ ${TLRENDER_USD} 	(TLRENDER_USD)"

if [[ $ASK_TO_CONTINUE == 1 ]]; then
    ask_to_continue
fi

#
# Handle Windows pre-flight compiles
#
if [[ $KERNEL == *Windows* ]]; then
    . $PWD/etc/windows/compile_dlls.sh
fi

if command -v swig > /dev/null 2>&1; then
    swig -version
else
    echo
    echo "swig NOT found!!! Cannot compile pyFLTK."
    echo
    exit 1
fi

#
# Work-around FLTK's CMakeLists.txt bug
#
rm -rf $BUILD_DIR/install/include/FL

cd $BUILD_DIR

#
# Handle Microsoft vcpkg variables
#
unset  VCPKG_ROOT
export VCPKG_INSTALL_PREFIX=$PWD/install

cmd="cmake -G '${CMAKE_GENERATOR}'
	   -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
           -D CMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
	   -D CMAKE_INSTALL_PREFIX=$PWD/install
	   -D CMAKE_PREFIX_PATH=$PWD/install
           -D CMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
           -D CMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
	   
	   -D NATIVE_C_COMPILER=\"${NATIVE_C_COMPILER}\"
	   -D GENERIC_C_COMPILER=\"${GENERIC_C_COMPILER}\"
	   -D GNU_C_COMPILER=\"${GNU_C_COMPILER}\"

	   -D NATIVE_CXX_COMPILER=\"${NATIVE_CXX_COMPILER}\"
	   -D GENERIC_CXX_COMPILER=\"${GENERIC_CXX_COMPILER}\"
	   -D GNU_CXX_COMPILER=\"${GNU_CXX_COMPILER}\"

	   -D NATIVE_CXX_COMPILER_NAME=\"${NATIVE_CXX_COMPILER_NAME}\"
	   -D GENERIC_CXX_COMPILER_NAME=\"${GENERIC_CXX_COMPILER_NAME}\"
	   -D GNU_CXX_COMPILER_NAME=\"${GNU_CXX_COMPILER_NAME}\"

	   -D NATIVE_C_COMPILER_NAME=\"${NATIVE_C_COMPILER_NAME}\"
	   -D GENERIC_C_COMPILER_NAME=\"${GENERIC_C_COMPILER_NAME}\"
	   -D GNU_C_COMPILER_NAME=\"${GNU_C_COMPILER_NAME}\"

	   -D BUILD_PYTHON=${BUILD_PYTHON}
	   -D BUILD_X11=${BUILD_X11}
	   -D BUILD_WAYLAND=${BUILD_WAYLAND}
	   -D BUILD_GETTEXT=${BUILD_GETTEXT}

	   -D MRV2_COMPILER=${COMPILER}
	   -D MRV2_BACKEND=${MRV2_BACKEND}
	   -D MRV2_CPPTRACE=${MRV2_CPPTRACE}
	   -D MRV2_DEMO=${MRV2_DEMO}
	   -D MRV2_HDR=${MRV2_HDR}
	   -D MRV2_NETWORK=${MRV2_NETWORK}
	   -D MRV2_PYFLTK=${MRV2_PYFLTK}
	   -D MRV2_PYBIND11=${MRV2_PYBIND11}
	   -D MRV2_PDF=${MRV2_PDF}

	   -D FLTK_BUILD_SHARED=${FLTK_BUILD_SHARED}

           -D TLRENDER_AOM=${TLRENDER_AOM}
           -D TLRENDER_API=${TLRENDER_API}
           -D TLRENDER_ASAN=${TLRENDER_ASAN}
           -D TLRENDER_AV1=${TLRENDER_AV1}
           -D TLRENDER_EXR=${TLRENDER_EXR}
           -D TLRENDER_FFMPEG=${TLRENDER_FFMPEG}
           -D TLRENDER_FFMPEG_MINIMAL=${TLRENDER_FFMPEG_MINIMAL}
	   -D TLRENDER_GL=${TLRENDER_GL}
           -D TLRENDER_HAP=${TLRENDER_HAP}
           -D TLRENDER_JPEG=${TLRENDER_JPEG}
           -D TLRENDER_LIBPLACEBO=${TLRENDER_LIBPLACEBO}
           -D TLRENDER_LOCAL=${TLRENDER_LOCAL}
	   -D TLRENDER_NDI=${TLRENDER_NDI}
	   -D TLRENDER_NDI_SDK=\"${TLRENDER_NDI_SDK}\"
	   -D TLRENDER_NET=${TLRENDER_NET}
	   -D TLRENDER_NFD=OFF
	   -D TLRENDER_OPENJPH=${TLRENDER_OPENJPH}
	   -D TLRENDER_RAW=${TLRENDER_RAW}
           -D TLRENDER_STB=${TLRENDER_STB}
           -D TLRENDER_SVTAV1=${TLRENDER_SVTAV1}
	   -D TLRENDER_TIFF=${TLRENDER_TIFF}
	   -D TLRENDER_USD=${TLRENDER_USD}
	   -D TLRENDER_VK=${TLRENDER_VK}
	   -D TLRENDER_VPX=${TLRENDER_VPX}
	   -D TLRENDER_WAYLAND=${TLRENDER_WAYLAND}
           -D TLRENDER_X11=${TLRENDER_X11}
           -D TLRENDER_X264=${TLRENDER_X264}
	   -D TLRENDER_YASM=${TLRENDER_YASM}
	   -D TLRENDER_PROGRAMS=OFF
	   -D TLRENDER_EXAMPLES=FALSE
	   -D TLRENDER_TESTS=FALSE
	   -D TLRENDER_USD_PYTHON=${TLRENDER_USD_PYTHON}
	   -D TLRENDER_QT6=OFF
	   -D TLRENDER_QT5=OFF
	   ${CMAKE_FLAGS} ../.."

run_cmd $cmd

run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE

cd -

if [[ "$CMAKE_TARGET" == "package" ]]; then
    #
    # When packaging, run mo target first.
    #
    export CMAKE_TARGET="mo"

    cmd="./runmeq.sh ${CMAKE_BUILD_TYPE} -t ${CMAKE_TARGET}"
    run_cmd $cmd

    #
    # Finally package it
    #
    export CMAKE_TARGET="package"
fi

if [[ "$CMAKE_TARGET" == "" ]]; then
    export CMAKE_TARGET="install"
fi

cmd="./runmeq.sh ${CMAKE_BUILD_TYPE} -t ${CMAKE_TARGET}"
run_cmd $cmd

if [[ "$CMAKE_TARGET" != "package" && "$CMAKE_TARGET" != "install" ]]; then
    . $PWD/etc/build_end.sh
fi

