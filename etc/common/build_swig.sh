#!/usr/bin/env bash

. etc/functions.sh

get_kernel

if [[ "$CMAKE_INSTALL_PREFIX" == "" ]]; then
    echo "Please set the CMAKE_INSTALL_PREFIX environment variable"
    exit 1
fi

swig_version=v4.4.0

current_dir=$PWD

cd /tmp
rm -rf swig

git clone https://github.com/swig/swig.git
cd swig
git checkout ${swig_version}

if [[ $KERNEL == "*Windows*" ]]; then
    #
    # Build it with MSys2
    #
    pacman -S --needed --noconfirm bison pcre gcc
    ./autogen.sh
    CC=gcc ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    make
    make install
    
    pacman -R bison pcre gcc --noconfirm
else
    #
    # Build it
    #
    ./autogen.sh
    ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    make
    make install
fi

hash -r

echo "SWIG version now:"
swig --version


cd $current_dir
