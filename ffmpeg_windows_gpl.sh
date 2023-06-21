#!/bin/bash

#
# This script compiles a GPL version of ffmpeg with libx264 support, but no
# libvpx support
#

. etc/build_dir.sh

ROOT_DIR=$PWD/$BUILD_DIR/gpl_ffmpeg
echo "GPL ffmpeg will be built in $ROOT_DIR"

rm -rf $ROOT_DIR

pacman -Syu --noconfirm
pacman -S make --noconfirm
pacman -S diffutils --noconfirm
pacman -S yasm --noconfirm
pacman -S nasm --noconfirm
pacman -S pkg-config --noconfirm

if [[ -e /usr/bin/link.exe ]]; then
    echo "Renaming /usr/bin/link.exe as /usr/bin/link_msys.exe"
    mv /usr/bin/link.exe /usr/bin/link_msys.exe
fi

mkdir -p $ROOT_DIR

cd    $ROOT_DIR
mkdir -p sources
mkdir -p build
mkdir -p install

#
# GETTING SOURCES
# 
cd sources
git clone --depth 1 https://code.videolan.org/videolan/x264.git
git clone --depth 1 git://source.ffmpeg.org/ffmpeg.git

#
# Fix x264 build scripts
#
cd x264
curl "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD" > config.guess
sed -i 's/host_os = mingw/host_os = msys/' configure


############
# BUILDING #
############

cd $ROOT_DIR/build


mkdir -p x264
cd x264
CC=cl ./../../sources/x264/configure --prefix=./../../installed --enable-shared
make -j ${CPU_CORES}
make install
mv ./../../installed/lib/libx264.dll.lib ./../../installed/lib/libx264.lib

cd $ROOT_DIR/build
mkdir -p ffmpeg
cd ffmpeg

export CC=cl
export PKG_CONFIG_PATH=$ROOT_DIR/installed/lib/pkgconfig

./../../sources/ffmpeg/configure \
             --prefix=./../../installed \
             --toolchain=msvc \
             --arch=x86_64 \
             --enable-x86asm \
             --enable-asm \
             --enable-shared \
             --disable-static \
             --disable-programs \
             --enable-swresample \
             --enable-libx264 \
             --enable-gpl \
             --extra-ldflags="-LIBPATH:./../../installed/lib/" \
             --extra-cflags="-I./../../installed/include/"

make -j ${CPU_CORES}
make install

#
# Built done
#
echo "GPL ffmpeg built done."
echo "Set the environment varialbe FFMPEG_ROOT=$ROOT_DIR/installed"

