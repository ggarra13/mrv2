#!/usr/bin/env bash

. etc/functions.sh

get_kernel

if [[ "$CMAKE_INSTALL_PREFIX" == "" ]]; then
    echo "Please set the CMAKE_INSTALL_PREFIX environment variable"
    exit 1
fi

swig_version=4.4.0

current_dir=$PWD

cd /tmp
rm -rf swig


if [[ $KERNEL == *Windows* ]]; then

    if [[ $ARCH == *amd64* ]]; then
	INSTALL_PREFIX=/ucrt64
	MSYS_ARCH=ucrt-x86_64
    else
	INSTALL_PREFIX=/clamgarm64
	MSYS_ARCH=clang-aarch64
    fi
    
    # #
    # # Build it with MSys2
    # #
    # if [[ ! -d swig-${swig_version} ]]; then
    # 	curl -LO https://downloads.sourceforge.net/project/swig/swig/swig-${swig_version}/swig-${swig_version}.tar.gz
    # 	tar -xzf swig-${swig_version}.tar.gz
    # fi
    
    # cd swig-${swig_version}

    # yes "" | pacman -Syu

    # yes "" | pacman -S --needed base-devel \
    #        mingw-w64-${MSYS_ARCH}-toolchain \
    #        mingw-w64-${MSYS_ARCH}-pcre2 \
    #        autoconf automake libtool bison --confirm

    # ./configure --prefix=$INSTALL_PREFIX
    # make
    # make install
else
    #
    # Build it
    #
    git clone https://github.com/swig/swig.git
    cd swig
    git checkout v${swig_version}

    ./autogen.sh
    ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    make
    make install
fi

hash -r

echo "SWIG version now:"
swig --version


cd $current_dir
