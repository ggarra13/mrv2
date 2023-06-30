#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script compiles a GPL or BSD version of ffmpeg. The GPL version has
# libx264 encoding and  libvpx support.  The BSD version does not have libx264.
#

. etc/build_dir.sh

if [[ $KERNEL != *Msys* ]]; then
    echo
    echo "This script is for Windows MSys2-64 only."
    echo
    exit 1
fi

export GPL=GPL
for i in $@; do
    case $i in
	-g|--gpl)
	    export GPL=GPL
	    shift
	    ;;
	-b|--bsd)
	    export GPL=BSD
	    shift
	    ;;
	-j)
	    shift
	    shift
	    ;;
	*)
	    echo
	    echo "Unknown parameter.  Usage is:"
	    echo
	    echo "$0 --gpl"
	    echo
	    echo "or:"
	    echo
	    echo "$0 --bsd"
	    exit 1
	    ;;
    esac
done

ROOT_DIR=$PWD/$BUILD_DIR/FFmpeg

if [[ $GPL == GPL ]]; then
    echo "GPL ffmpeg will be built in $ROOT_DIR"
else
    echo "BSD ffmpeg will be built in $ROOT_DIR"
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
if [[ $GPL == BSD ]]; then
    export BUILD_LIBX264=0
fi

#
# Build FFMPEG with the GPL libraries specified above.
#
export BUILD_FFMPEG=1

if [[ $MSYS_LIBS == 1 ]]; then

    echo ""
    echo "If you are asked to close the terminal, you should do so and"
    echo "run this script again."
    echo ""
    pacman -Syu --noconfirm

    #
    # This is for libx264 and ffmpeg
    #
    echo "Installing packages needed to build libx264 and ffmpeg..."
    pacman -S make diffutils yasm nasm pkg-config --noconfirm

    if [[ -e /usr/bin/link.exe ]]; then
	echo "Renaming /usr/bin/link.exe as /usr/bin/link_msys.exe"
	mv /usr/bin/link.exe /usr/bin/link_msys.exe
    fi

    echo "Cleaning $ROOT_DIR...."
    rm -rf $ROOT_DIR

fi


mkdir -p $ROOT_DIR

cd    $ROOT_DIR

mkdir -p sources
mkdir -p build
mkdir -p install


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
    
    if [[ ! -e $ROOT_DIR/install/lib/vpx.lib ]]; then
	cd $ROOT_DIR/build
	mkdir -p libvpx
	cd libvpx
    
	./../../sources/libvpx/configure --prefix=./../../install --target=x86_64-win64-vs16 --disable-examples --disable-docs 
	make -j ${CPU_CORES}
	make install
	mv $ROOT_DIR/install/lib/x64/vpxmd.lib $ROOT_DIR/install/lib/vpx.lib 
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
	git clone --depth 1 https://code.videolan.org/videolan/x264.git
    fi
    
    #
    # Fix x264 build scripts
    #
    cd x264
    curl "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD" > config.guess
    sed -i 's/host_os = mingw/host_os = msys/' configure
    
    cd $ROOT_DIR/build
    mkdir -p x264
    cd x264
    CC=cl ./../../sources/x264/configure --prefix=./../../install --enable-shared
    make -j ${CPU_CORES}
    make install
    mv ./../../install/lib/libx264.dll.lib ./../../install/lib/libx264.lib
    export ENABLE_LIBX264="--enable-libx264"
fi

#
# Build ffmpeg
#

if [[ $BUILD_FFMPEG == 1 ]]; then
    cd $ROOT_DIR/sources

    if [[ ! -d ffmpeg ]]; then
	git clone --depth 1 git://source.ffmpeg.org/ffmpeg.git --branch n6.0
    fi
    
    cd $ROOT_DIR/build
    mkdir -p ffmpeg
    cd ffmpeg

    export CC=cl
    export PKG_CONFIG_PATH=$ROOT_DIR/install/lib/pkgconfig

    echo "ENABLE_LIBVPX=${ENABLE_LIBVPX}"
    
    ./../../sources/ffmpeg/configure \
        --prefix=./../../install \
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
        --enable-gpl \
        --extra-ldflags="-LIBPATH:./../../install/lib/" \
        --extra-cflags="-I./../../install/include/"

    make -j ${CPU_CORES}
    make install
fi


if [[ $MSYS_LIBS == 1 ]]; then
    #
    # Remove the libx264 and ffmpeg libs we downloaded
    #
    pacman -R make diffutils yasm nasm pkg-config msys2-runtime-devel libxml2-devel  zlib-devel --noconfirm
fi

#
# Built done
#
echo "${GPL} ffmpeg built done."
echo ""
echo "Run the mrv2 compileation again with:"
echo ""
echo "runme.sh"
echo ""
