#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script compiles a GPL or BSD version of ffmpeg. The GPL version has
# libx264 encoding and libvpx support.  The LGPL version does not have libx264.
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
LIBVPX_TAG=v1.12.0
DAV1D_TAG=1.3.0
SVTAV1_TAG=v1.8.0
X264_TAG=stable
FFMPEG_TAG=n6.0

SVTAV1_REPO=https://gitlab.com/AOMediaCodec/SVT-AV1.git

#
# Some auxiliary variables
#
MRV2_ROOT=$PWD
ROOT_DIR=$PWD/$BUILD_DIR/tlRender/etc/SuperBuild/FFmpeg
INSTALL_DIR=$PWD/$BUILD_DIR/install

if [[ $FFMPEG_GPL == GPL || $FFMPEG_GPL == LGPL ]]; then
    true
else
    echo
    echo "No --gpl or --lgpl flag provided to compile FFmpeg.  Choosing --lgpl."
    echo
    export FFMPEG_GPL=LGPL
fi

if [ -z "$TLRENDER_FFMPEG" ]; then
    export TLRENDER_FFMPEG=ON
fi

lgpl_ffmpeg=""
if [[ -e $INSTALL_DIR/lib/avformat.lib ]]; then
    avformat_dll=`ls $INSTALL_DIR/bin/avformat*.dll`
    lgpl_ffmpeg=`strings $avformat_dll | findstr "LGPL" | grep -o "LGPL"`
fi

if [[ "$lgpl_ffmpeg" != "" ]]; then
    if [[ $lgpl_ffmpeg != $FFMPEG_GPL ]]; then
	echo "Incompatible FFmpeg already installed.  Installed is $lgpl_ffmpeg, want $FMMPEG_GPL."
	run_cmd rm -rf $INSTALL_DIR/lib/avformat.lib
    else
	echo "Compatible FFmpeg already installed."
	export TLRENDER_FFMPEG=OFF
    fi
fi

    
if [[ $TLRENDER_FFMPEG == ON || $TLRENDER_FFMPEG == 1 ]]; then
    pacman -Sy make wget diffutils yasm nasm pkg-config --noconfirm
fi
    

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
# Build with h264 encoding.
#
BUILD_LIBX264=1
if [[ $FFMPEG_GPL == LGPL ]]; then
    BUILD_LIBX264=OFF
fi

if [[ $TLRENDER_FFMPEG == OFF || $TLRENDER_FFMPEG == 0 ]]; then
    export TLRENDER_VPX=OFF
    export TLRENDER_AV1=OFF
    export TLRENDER_NET=OFF
    export BUILD_LIBX264=OFF
else
    echo
    echo "Installing packages needed to build:"
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
    if [[ $FFMPEG_GPL == GPL ]]; then
	echo "libx264"
    fi
    echo "FFmpeg"
    echo
fi

#
# Build with libdav1d decoder and SVT-AV1 encoder.
#
if [[ $TLRENDER_AV1 == ON || $TLRENDER_AV1 == 1 ]]; then
    BUILD_LIBDAV1D=1
    if [[ $has_meson == 0 ]]; then
	echo "Please install meson from https://github.com/mesonbuild/meson/releases"
	BUILD_LIBDAV1D=0
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



#############
## BUILDING #
#############


## Build libvpx
ENABLE_LIBVPX=""
if [[ $TLRENDER_VPX == ON || $TLRENDER_VPX == 1 ]]; then
    cd $ROOT_DIR/sources
    if [[ ! -d libvpx ]]; then
	git clone --depth 1 --branch ${LIBVPX_TAG} https://chromium.googlesource.com/webm/libvpx 2> /dev/null
    fi
    
    if [[ ! -e $INSTALL_DIR/lib/vpx.lib ]]; then
	cd $ROOT_DIR/build
	mkdir -p libvpx
	cd libvpx
    
	echo
	echo "Compiling libvpx......"
	echo

	
	# ./../../sources/libvpx/configure --prefix=$INSTALL_DIR \
	# 				 --target=x86_64-win64-vs16 \
	# 				 --disable-examples \
	# 				 --disable-docs \
	# 				 --disable-unit-tests \
	# 				 --disable-debug \
	# 				 --log=no \
	# 				 --disable-debug-libs \
	# 				 --disable-dependency-tracking

	unset CC
	unset CXX
	unset LD
	./../../sources/libvpx/configure --prefix=$INSTALL_DIR \
					 --target=x86_64-win64-vs16 \
					 --enable-vp9-highbitdepth \
					 --disable-unit-tests \
					 --disable-examples \
					 --disable-docs
	make -j ${CPU_CORES}
	make install
	run_cmd mv $INSTALL_DIR/lib/x64/vpxmd.lib $INSTALL_DIR/lib/vpx.lib
	run_cmd rm -rf $INSTALL_DIR/lib/x64/
    fi
    
    ENABLE_LIBVPX='--enable-libvpx --extra-libs=vpx.lib --extra-libs=kernel32.lib --extra-libs=user32.lib --extra-libs=gdi32.lib --extra-libs=winspool.lib --extra-libs=shell32.lib --extra-libs=ole32.lib --extra-libs=oleaut32.lib --extra-libs=uuid.lib --extra-libs=comdlg32.lib --extra-libs=advapi32.lib --extra-libs=msvcrt.lib'
fi


#
# Build libdav1d decoder
#
ENABLE_LIBDAV1D=""
if [[ $BUILD_LIBDAV1D == 1 ]]; then

    if [[ $has_pip3 == 1 ]]; then
	pip3 install meson
	has_meson=1
    fi
    
    cd $ROOT_DIR/sources

    if [[ ! -d dav1d ]]; then
	git clone --depth 1 https://code.videolan.org/videolan/dav1d.git --branch ${DAV1D_TAG} 2> /dev/null
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
    
    ENABLE_LIBDAV1D="--enable-libdav1d"
fi

#
# Build libSvt-AV1 encoder
#
ENABLE_LIBSVTAV1=""
if [[ $BUILD_LIBSVTAV1 == 1 ]]; then
    
    cd $ROOT_DIR/sources

    if [[ ! -d SVT-AV1 ]]; then
	echo "Cloning ${SVTAV1_REPO}"
	git clone --depth 1 ${SVTAV1_REPO} --branch ${SVTAV1_TAG} 2> /dev/null

	# We need to download a win64 specific yasm, not msys64 one
	wget -c http://www.tortall.net/projects/yasm/releases/yasm-1.3.0-win64.exe
	mv yasm-1.3.0-win64.exe yasm.exe
	
    fi

    if [[ ! -e $INSTALL_DIR/lib/SvtAV1Enc.lib ]]; then
	echo "Building SvtAV1Enc.lib"
	cd SVT-AV1
	export OLD_PATH=$PATH

	export PATH=$ROOT_DIR/sources:$PATH
	
	cd Build/windows
	cmd //c build.bat 2019 release static no-dec no-apps

	export PATH=$OLD_PATH

	cd -
	
	cp Bin/Release/SvtAv1Enc.lib $INSTALL_DIR/lib

	mkdir -p $INSTALL_DIR/include/svt-av1
	cp Source/API/*.h $INSTALL_DIR/include/svt-av1
	
	cd $ROOT_DIR/sources
	rm yasm.exe
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
    
    ENABLE_LIBSVTAV1="--enable-libsvtav1"
fi


#
# Build x264
#
ENABLE_LIBX264=""
if [[ $BUILD_LIBX264 == 1 ]]; then
    
    cd $ROOT_DIR/sources

    if [[ ! -d x264 ]]; then
	git clone --depth 1 https://code.videolan.org/videolan/x264.git --branch ${X264_TAG} 2> /dev/null
    fi

    if [[ ! -e $INSTALL_DIR/lib/libx264.lib ]]; then
	echo
	echo "Compiling libx264 as $FFMPEG_GPL......"
	echo
	cd $ROOT_DIR/build
	mkdir -p x264
	cd x264
	CC=cl ./../../sources/x264/configure --prefix=$INSTALL_DIR --enable-shared
	make -j ${CPU_CORES}
	make install
	run_cmd mv $INSTALL_DIR/lib/libx264.dll.lib $INSTALL_DIR/lib/libx264.lib
    fi
    
    ENABLE_LIBX264="--enable-libx264 --enable-gpl"
    if [[ $TLRENDER_NET == 1 ]]; then
	ENABLE_LIBX264="${ENABLE_LIBX264} --enable-version3"
    fi
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
ENABLE_OPENSSL=""
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
    ENABLE_OPENSSL="--enable-openssl --extra-libs=crypto.lib"
fi

echo "Preparing to build FFmpeg...."

#
# Build FFmpeg
#

if [[ $TLRENDER_FFMPEG == ON || $TLRENDER_FFMPEG == 1 ]]; then
    cd $ROOT_DIR/sources

    if [[ ! -d ffmpeg ]]; then
	echo "Cloning ffmpeg repository..."
	git clone --depth 1 --branch ${FFMPEG_TAG} https://git.ffmpeg.org/ffmpeg.git 2> /dev/null
    fi
    
    if [[ ! -e $INSTALL_DIR/lib/avformat.lib ]]; then
	echo
	echo "Compiling FFmpeg as ${FFMPEG_GPL}......"
	echo
	run_cmd rm -rf $ROOT_DIR/build/ffmpeg
	cd $ROOT_DIR/build
	mkdir -p ffmpeg
	cd ffmpeg
	
	if [[ -e $INSTALL_DIR/include/zconf.h ]]; then
	    mv $INSTALL_DIR/include/zconf.h $INSTALL_DIR/include/zconf.h.orig
	fi

	export CC=cl
	export PKG_CONFIG_PATH=$INSTALL_DIR/lib/pkgconfig

	# -wd4828 disables non UTF-8 characters found on non-English MSVC
	# -wd4101 disables local variable without reference
	# -wd4267 disables conversion from size_t to int, possible loss of data
	# -wd4334 '<<': result of 32-bit shift implicitly converted to 64 bits
	# -wd4090 'function': different 'const' qualifiers
	./../../sources/ffmpeg/configure \
            --prefix=$INSTALL_DIR \
	    --pkg-config-flags=--static \
            --disable-programs \
            --disable-doc \
            --disable-postproc \
            --disable-avfilter \
            --disable-hwaccels \
            --disable-devices \
            --disable-filters \
            --disable-alsa \
            --disable-appkit \
            --disable-avfoundation \
            --disable-bzlib \
            --disable-coreimage \
            --disable-iconv \
            --disable-libxcb \
            --disable-libxcb-shm \
            --disable-libxcb-xfixes \
            --disable-libxcb-shape \
            --disable-lzma \
            --disable-metal \
            --disable-sndio \
            --disable-schannel \
            --disable-sdl2 \
            --disable-securetransport \
            --disable-vulkan \
            --disable-xlib \
            --disable-zlib \
            --disable-amf \
            --disable-audiotoolbox \
            --disable-cuda-llvm \
            --disable-cuvid \
            --disable-d3d11va \
            --disable-dxva2 \
            --disable-ffnvcodec \
            --disable-nvdec \
            --disable-nvenc \
            --disable-v4l2-m2m \
            --disable-vaapi \
            --disable-vdpau \
            --disable-videotoolbox \
            --enable-pic \
            --toolchain=msvc \
            --target-os=win64 \
            --arch=x86_64 \
            --enable-x86asm \
            --enable-asm \
            --enable-shared \
            --disable-static \
            --enable-swresample \
            $ENABLE_OPENSSL \
            $ENABLE_LIBX264 \
            $ENABLE_LIBDAV1D \
            $ENABLE_LIBSVTAV1 \
            $ENABLE_LIBVPX \
            --extra-ldflags="-LIBPATH:$INSTALL_DIR/lib/" \
            --extra-cflags="-I$INSTALL_DIR/include/ -MD -wd4828 -wd4101 -wd4267 -wd4334 -wd4090"

        make -j ${CPU_CORES}
        make install

        # @bug: FFmpeg places .lib in bin/
        run_cmd mv $INSTALL_DIR/bin/*.lib $INSTALL_DIR/lib/

        if [[ -e $INSTALL_DIR/include/zconf.h.orig ]]; then
            run_cmd mv $INSTALL_DIR/include/zconf.h.orig $INSTALL_DIR/include/zconf.h
        fi
    fi
fi


if [[ $TLRENDER_FFMPEG == ON || $TLRENDER_FFMPEG == 1 ]]; then
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
    if [[ $FFMPEG_GPL == GPL ]]; then
	echo "libx264"
    fi
    echo "FFmpeg"
    echo
    pacman -R yasm nasm --noconfirm
fi

cd $MRV2_ROOT
