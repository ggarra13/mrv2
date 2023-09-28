#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script compiles a GPL or BSD version of ffmpeg. The GPL version has
# libx264 encoding and libvpx support.  The BSD version does not have libx264.
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
fi

if [[ $KERNEL != *Msys* ]]; then
    echo
    echo "This script is for Windows MSys2-64 only."
    echo
    exit 1
fi

export LIBVPX_TAG=v1.12.0
export X264_TAG=master
export FFMPEG_TAG=n6.0

ROOT_DIR=$PWD/$BUILD_DIR/tlRender/etc/SuperBuild/FFmpeg
export INSTALL_DIR=$PWD/$BUILD_DIR/install

if [[ $FFMPEG_GPL == GPL || $FFMPEG_GPL == LGPL ]]; then
    true
else
    echo
    echo "You need to provide either a --gpl or --bsd flag."
    exit 1
fi
#
# This configures the environment for compilation.  It also cleans at the
# end to leave it ready for mrv2 build.
#
export MSYS_LIBS=1

#
# Build with libvpx (webm) movies.
#
export BUILD_LIBVPX=1

#
# Build wiht h264 encoding.
#
export BUILD_LIBX264=1
if [[ $FFMPEG_GPL == LGPL ]]; then
    export BUILD_LIBX264=0
fi

#
# Build FFMPEG with the GPL libraries specified above.
#
export BUILD_FFMPEG=1



if [[ $MSYS_LIBS == 1 ]]; then
    pacman -Sy --noconfirm

    #
    # This is for libx264 and ffmpeg
    #
    if [[ $FFMPEG_GPL == GPL ]]; then
	echo
	echo "Installing packages needed to build libvpx, libx264 and FFmpeg..."
    else
	echo
	echo "Installing packages needed to build libvpx and FFmpeg..."
    fi
    echo
    
    pacman -Sy make diffutils yasm nasm pkg-config --noconfirm

fi

mkdir -p $ROOT_DIR

cd    $ROOT_DIR

mkdir -p sources
mkdir -p build



#############
## BUILDING #
#############

## Build libvpx
export ENABLE_LIBVPX=""
if [[ $BUILD_LIBVPX == 1 ]]; then
    cd $ROOT_DIR/sources
    if [[ ! -d libvpx ]]; then
	git clone --depth 1 https://chromium.googlesource.com/webm/libvpx
    fi
    
    if [[ ! -e $INSTALL_DIR/lib/vpx.lib ]]; then
	cd $ROOT_DIR/build
	mkdir -p libvpx
	cd libvpx
    
	echo
	echo "Compiling libvpx as $FFMPEG_GPL......"
	echo

	
	./../../sources/libvpx/configure --prefix=$INSTALL_DIR --target=x86_64-win64-vs16 --disable-examples --disable-docs 
	make -j ${CPU_CORES}
	make install
	mv $INSTALL_DIR/lib/x64/vpxmd.lib $INSTALL_DIR/lib/vpx.lib
	rm -rf $INSTALL_DIR/lib/x64/
    fi
    
    export ENABLE_LIBVPX='--enable-libvpx --extra-libs=vpx.lib --extra-libs=kernel32.lib --extra-libs=user32.lib --extra-libs=gdi32.lib --extra-libs=winspool.lib --extra-libs=shell32.lib --extra-libs=ole32.lib --extra-libs=oleaut32.lib --extra-libs=uuid.lib --extra-libs=comdlg32.lib --extra-libs=advapi32.lib --extra-libs=msvcrt.lib'
fi


#
# Build x264
#
export ENABLE_LIBX264=""
if [[ $BUILD_LIBX264 == 1 ]]; then
    
    cd $ROOT_DIR/sources

    if [[ ! -d x264 ]]; then
	git clone --depth 1 https://code.videolan.org/videolan/x264.git --branch ${X264_TAG}
	#
	# Fix x264 build scripts
	#
	cd x264
	curl "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD" > config.guess
	sed -i 's/host_os = mingw/host_os = msys/' configure
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
	mv $INSTALL_DIR/lib/libx264.dll.lib $INSTALL_DIR/lib/libx264.lib
    fi
    
    export ENABLE_LIBX264="--enable-libx264 --enable-gpl"
else
    # Remove unused libx264
    rm -f $INSTALL_DIR/bin/libx264*.dll
    rm -f $INSTALL_DIR/lib/libx264.lib
fi

#
# Build ffmpeg
#

if [[ $BUILD_FFMPEG == 1 ]]; then
    echo "Building FFmpeg..."
    cd $ROOT_DIR/sources

    if [[ ! -d ffmpeg ]]; then
	git clone --depth 1 --branch ${FFMPEG_TAG} git://source.ffmpeg.org/ffmpeg.git
    fi
    
    if [[ ! -e $INSTALL_DIR/lib/avformat.lib ]]; then
	echo
	echo "Compiling FFmpeg as ${FFMPEG_GPL}......"
	echo
	rm -rf $ROOT_DIR/build/ffmpeg
	cd $ROOT_DIR/build
	mkdir -p ffmpeg
	cd ffmpeg
	
	if [[ -e $INSTALL_DIR/include/zconf.h ]]; then
	    mv $INSTALL_DIR/include/zconf.h $INSTALL_DIR/include/zconf.h.orig
	fi

	export CC=cl
	export PKG_CONFIG_PATH=$INSTALL_DIR/lib/pkgconfig

	./../../sources/ffmpeg/configure \
            --prefix=$INSTALL_DIR \
            --toolchain=msvc \
	    --target-os=win64 \
            --arch=x86_64 \
            --enable-x86asm \
            --enable-asm \
            --enable-shared \
            --disable-static \
            --disable-programs \
            --enable-swresample \
            $ENABLE_LIBX264 \
	    $ENABLE_LIBVPX \
            --extra-ldflags="-LIBPATH:$INSTALL_DIR/lib/" \
            --extra-cflags="-I$INSTALL_DIR/include/"

	# @bug: MSVC creates bad characters on MSVC with different languages but
	# we can't fix it
	make -j ${CPU_CORES}
	make install

	# @bug: FFmpeg places .lib in bin/
	mv $INSTALL_DIR/bin/*.lib $INSTALL_DIR/lib/

	if [[ -e $INSTALL_DIR/include/zconf.h.orig ]]; then
	   mv $INSTALL_DIR/include/zconf.h.orig $INSTALL_DIR/include/zconf.h
	fi
    fi
fi


if [[ $MSYS_LIBS == 1 ]]; then
    echo "Removing packages used to build libx264 and FFmpeg..."
    pacman -R yasm nasm --noconfirm
fi

echo "Exiting..."
