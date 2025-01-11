#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script compiles a GPL or LGPL version of ffmpeg. The GPL version has
# libx264 encoding support.  The LGPL version does not have libx264.
#
if [[ ! -e etc/build_dir.sh ]]; then
    echo "You must run this script from the root of mrv2 directory like:"
    echo
    script=`basename $0`
    echo "> bin/$script"
    exit 1
fi

if [[ ! $RUNME ]]; then
    . etc/build_dir.sh
else
    . etc/functions.sh
    
    get_msvc_version
fi

if [[ $KERNEL != *Msys* ]]; then
    echo
    echo "This script is for Windows MSys2-64 only."
    echo
    exit 1
fi

#
# Latest TAGS of all libraries
#
#LIBVPX_TAG=6f0c446c7b88d384a1c09caf33ec132e7ee24aea
LIBVPX_TAG=7a65480684b1b28bb9defae164bf0dc78b32653e
DAV1D_TAG=1.3.0
SVTAV1_TAG=v2.1.2
X264_TAG=stable


#
# Repositories
#
YASM_TGZ=http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz
SVTAV1_REPO=https://gitlab.com/AOMediaCodec/SVT-AV1.git
LIBVPX_REPO=https://chromium.googlesource.com/webm/libvpx.git
LIBDAV1D_REPO=https://code.videolan.org/videolan/dav1d.git 
LIBX264_REPO=https://code.videolan.org/videolan/x264.git

#
# Some auxiliary variables
#
MRV2_ROOT=$PWD
ROOT_DIR=$PWD/$BUILD_DIR/tlRender/etc/SuperBuild/FFmpeg
INSTALL_DIR=$PWD/$BUILD_DIR/install
    

has_pip3=0
if type -P pip3 &> /dev/null; then
    has_pip3=1
fi

has_meson=0
if type -P meson &> /dev/null; then
    has_meson=1
fi

has_cmake=0
if type -P cmake &> /dev/null; then
    has_cmake=1
fi

if [ -z "$TLRENDER_AV1" ]; then
    export TLRENDER_AV1=ON
fi

if [ -z "$TLRENDER_NET" ]; then
    export TLRENDER_NET=ON
fi

if [ -z "$TLRENDER_VPX" ]; then
    export TLRENDER_VPX=ON
fi

#
# Get Msys dependencies
#
pacman -Sy make wget diffutils nasm pkg-config --noconfirm

#
# Build with h264 encoding.
#
TLRENDER_X264=ON
if [[ $FFMPEG_GPL == LGPL ]]; then
    TLRENDER_X264=OFF
fi

if [[ $TLRENDER_FFMPEG == OFF || $TLRENDER_FFMPEG == 0 ]]; then
    export TLRENDER_VPX=OFF
    export TLRENDER_AV1=OFF
    export TLRENDER_NET=OFF
    export TLRENDER_X264=OFF
else
    echo
    echo "Installing packages needed to build:"
    echo
    if [[ $TLRENDER_VPX == ON || $TLRENDER_VPX == 1 ]]; then
	echo "libvpx"
    fi
    if [[ $TLRENDER_AV1 == ON || $TLRENDER_AV1 == 1 ]]; then
	if [[ $has_meson || $has_pip3 ]]; then
	    echo "libdav1d"
	fi
	if [[ $has_cmake == 1 ]]; then
	    echo "libSvtAV1"
	fi
    fi
    if [[ $TLRENDER_X264 == ON || $TLRENDER_X264 == 1 ]]; then
	echo "libx264"
    fi
    echo
fi

#
# Build with libdav1d decoder and SVT-AV1 encoder.
#
if [[ $TLRENDER_AV1 == ON || $TLRENDER_AV1 == 1 ]]; then
    BUILD_LIBDAV1D=1
    if [[ $has_meson == 0 ]]; then
	if [[ $has_pip3 == 1 ]]; then
	    pip3 install meson
	    has_meson=1
	    BUILD_LIBDAV1D=1
	else
	    echo "Please install meson from https://github.com/mesonbuild/meson/releases"
	    BUILD_LIBDAV1D=0
	fi
    fi

    BUILD_LIBSVTAV1=1
    if [[ $has_cmake == 0 ]]; then
	BUILD_LIBSVTAV1=0
    fi
fi

mkdir -p $ROOT_DIR

cd    $ROOT_DIR

mkdir -p sources
mkdir -p build

download_yasm() {
    cd $ROOT_DIR/sources

    if [ ! -e yasm.exe ]; then
	wget -c http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz
	tar -xf yasm-1.3.0.tar.gz
	cd yasm-1.3.0
	mkdir build
	cd build
	cmake .. -G Ninja -D CMAKE_INSTALL_PREFIX=$INSTALL_DIR -D CMAKE_BUILD_TYPE=Release -D BUILD_SHARED_LIBS=OFF -D YASM_BUILD_TESTS=OFF
	ninja
	mv yasm.exe ../..
	# 	# We need to download a win64 specific yasm, not msys64 one
	# 	wget -c https://github.com/yasm/yasm/releases/download/v1.3.0/yasm-1.3.0-win64.exe
	# 	mv yasm-1.3.0-win64.exe yasm.exe
    fi
}


export OLD_PATH="$PATH"

#
# Add current path to avoid long PATH on Windows.
#
export PATH=".:$OLD_PATH"

if [[ $TLRENDER_VPX == ON || $TLRENDER_VPX == 1 || \
	  $BUILD_LIBSVTAV1 == 1 ]]; then
    download_yasm
fi

#############
## BUILDING #
#############


## Build libvpx
if [[ $TLRENDER_VPX == ON || $TLRENDER_VPX == 1 ]]; then
    cd $ROOT_DIR/sources
    if [[ ! -d libvpx ]]; then
	git clone ${LIBVPX_REPO}
    fi
    
    if [[ ! -e $INSTALL_DIR/lib/vpx.lib ]]; then

	cd $ROOT_DIR/sources/libvpx
	git checkout ${LIBVPX_TAG}
    
	echo
	echo "Compiling libvpx......"
	echo

	cp $ROOT_DIR/sources/yasm.exe .

	target=x86_64-win64-vs17
	./configure --prefix=$INSTALL_DIR \
		    --target=$target \
		    --enable-vp9-highbitdepth \
		    --disable-unit-tests \
		    --disable-examples \
		    --disable-docs
	make -j ${CPU_CORES}
	make install
	run_cmd mv $INSTALL_DIR/lib/x64/vpxmd.lib $INSTALL_DIR/lib/vpx.lib
	run_cmd rm -rf $INSTALL_DIR/lib/x64/
    fi
fi


#
# Build libdav1d decoder
#
ENABLE_LIBDAV1D=""
if [[ $BUILD_LIBDAV1D == 1 ]]; then
    
    cd $ROOT_DIR/sources

    if [[ ! -d dav1d ]]; then
	git clone --depth 1 ${LIBDAV1D_REPO} --branch ${DAV1D_TAG}
    fi

    if [[ ! -e $INSTALL_DIR/lib/dav1d.lib ]]; then
	echo
	echo "Compiling libdav1d......"
	echo
	cd dav1d
	export CC=cl.exe
	meson setup -Denable_tools=false -Denable_tests=false --default-library=static -Dlibdir=$INSTALL_DIR/lib --prefix=$INSTALL_DIR build
	cd build
	ninja -j ${CPU_CORES}
	ninja install
	run_cmd mv $INSTALL_DIR/lib/libdav1d.a $INSTALL_DIR/lib/dav1d.lib 
    fi
fi

#
# Build libSvt-AV1 encoder
#
if [[ $BUILD_LIBSVTAV1 == 1 ]]; then
    
    cd $ROOT_DIR/sources

    if [[ ! -d SVT-AV1 ]]; then
	echo "Cloning ${SVTAV1_REPO}"
	git clone --depth 1 ${SVTAV1_REPO} --branch ${SVTAV1_TAG}
    fi

    if [[ ! -e $INSTALL_DIR/lib/SvtAV1Enc.lib ]]; then
	echo "Building SvtAV1Enc.lib"
	cd SVT-AV1
	
	cp $ROOT_DIR/sources/yasm.exe .
	

	cd Build/windows
	
	cmd //c build.bat ${MSVC_VERSION} release static no-apps

	cd -
	
	cp Bin/Release/SvtAv1Enc.lib $INSTALL_DIR/lib

	mkdir -p $INSTALL_DIR/include/svt-av1
	cp Source/API/*.h $INSTALL_DIR/include/svt-av1
    fi

    if [[ ! -e $INSTALL_DIR/lib/pkgconfig/SvtAv1Enc.pc ]]; then

	mkdir -p $INSTALL_DIR/lib/pkgconfig
	cd $INSTALL_DIR/lib/pkgconfig
	
	cat <<EOF > SvtAv1Enc.pc
prefix=$INSTALL_DIR
includedir=\${prefix}/include
libdir=\${prefix}/lib

Name: SvtAv1Enc
Description: SVT (Scalable Video Technology) for AV1 encoder library
Version: 1.8.0
Libs: -L\${libdir} -lSvtAv1Enc
Cflags: -I\${includedir}/svt-av1
Cflags.private: -UEB_DLL
EOF

	sed -i -e 's#([A-Z])\:#/\\1/#' SvtAv1Enc.pc

	cd $ROOT_DIR/sources
    fi
fi


#
# Build x264
#
ENABLE_LIBX264=""
if [[ $TLRENDER_X264 == ON || $TLRENDER_X264 == 1 ]]; then
    
    cd $ROOT_DIR/sources

    if [[ ! -d x264 ]]; then
	git clone ${LIBX264_REPO} --branch ${X264_TAG}
    fi

    if [[ ! -e $INSTALL_DIR/lib/libx264.lib ]]; then
	echo
	echo "Compiling libx264 as GPL......"
	echo
	cd $ROOT_DIR/build
	mkdir -p x264
	cd x264
	CC=cl ./../../sources/x264/configure --prefix=$INSTALL_DIR --enable-shared
	make -j ${CPU_CORES}
	make install
	run_cmd mv $INSTALL_DIR/lib/libx264.dll.lib $INSTALL_DIR/lib/libx264.lib
    fi
    
    ENABLE_LIBX264="--enable-libx264 
                    --enable-decoder=libx264
                    --enable-encoder=libx264
                    --enable-gpl"
else
    # Remove unused libx264
    if [[ -e $INSTALL_DIR/lib/libx264.lib ]]; then
	run_cmd rm -f $INSTALL_DIR/bin/libx264*.dll
	run_cmd rm -f $INSTALL_DIR/lib/libx264.lib
    fi
fi


#
# Install openssl and libcrypto
#
if [[ $TLRENDER_NET == ON || $TLRENDER_NET == 1 ]]; then
    if [[ ! -e $BUILD_DIR/install/lib/ssl.lib ]]; then
	
	if [[ ! -e /mingw64/lib/libssl.dll.a ]]; then
	    pacman -Sy mingw-w64-x86_64-openssl --noconfirm
	fi

	run_cmd cp /mingw64/bin/libssl*.dll $INSTALL_DIR/bin/
	run_cmd cp /mingw64/lib/libssl.dll.a $INSTALL_DIR/lib/ssl.lib
	run_cmd cp /mingw64/bin/libcrypto*.dll $INSTALL_DIR/bin/
	run_cmd cp /mingw64/lib/libcrypto.dll.a $INSTALL_DIR/lib/crypto.lib
	run_cmd mkdir -p $INSTALL_DIR/lib/pkgconfig
	run_cmd cp /mingw64/lib/pkgconfig/libssl.pc $INSTALL_DIR/lib/pkgconfig/openssl.pc

	run_cmd cp -r /mingw64/include/openssl $INSTALL_DIR/include/
	run_cmd sed -i -e 's/SSL_library_init../SSL_library_init/' $INSTALL_DIR/include/openssl/ssl.h
	run_cmd sed -i -e "s#=/mingw64#=$INSTALL_DIR#" $INSTALL_DIR/lib/pkgconfig/openssl.pc
	run_cmd sed -i -e 's%Requires.private:.libcrypto%%' $INSTALL_DIR/lib/pkgconfig/openssl.pc
	
	run_cmd cp /mingw64/lib/pkgconfig/libcrypto.pc $INSTALL_DIR/lib/pkgconfig/
	run_cmd sed -i -e "s#=/mingw64#=$INSTALL_DIR#" $INSTALL_DIR/lib/pkgconfig/libcrypto.pc
    fi
fi

echo
echo "Removing packages used to build:"
echo
if [[ $TLRENDER_VPX == ON || $TLRENDER_VPX == 1 ]]; then
    echo "libvpx"
fi
if [[ $TLRENDER_AV1 == ON || $TLRENDER_AV1 == 1 ]]; then
    if [[ $has_meson == 1 || $has_pip3 ]]; then
	echo "libdav1d"
    fi
    if [[ $has_cmake == 1 ]]; then
	echo "libSvtAV1"
    fi
fi
if [[ $TLRENDER_X264 == ON || $TLRENDER_X264 == 1 ]]; then
    echo "libx264"
fi

#
# Restore PATH
#
export PATH="$OLD_PATH"

#
# Set PKG_CONFIG_PATH 
#
export PKG_CONFIG_PATH=$INSTALL_DIR/lib/pkgconfig:$PKG_CONFIG_PATH

cd $MRV2_ROOT
