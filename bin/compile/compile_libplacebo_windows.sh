#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script compiles a libplacebo as a shared library (DLL).
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

#
# Set the main tag to compile
#
LIBPLACEBO_BRANCH=master

if [[ $KERNEL != *Msys* ]]; then
    echo
    echo "This script is for Windows MSys2-64 only."
    echo
    exit 1
fi

if [ -z "$TLRENDER_LIBPLACEBO" ]; then
    export TLRENDER_LIBPLACEBO=ON
fi

MRV2_ROOT=$PWD
SUPERBUILD=$PWD/$BUILD_DIR/tlRender/etc/SuperBuild
INSTALLDIR=$PWD/$BUILD_DIR/install

if [[ $TLRENDER_LIBPLACEBO == ON || $TLRENDER_LIBPLACEBO == 1 ]]; then
    
    if [[ ! -e $INSTALLDIR/lib/libplacebo.lib ]]; then
	
	#
	# Install development tools
	#
	pacman -Sy --noconfirm

	pacman -Sy mingw-w64-x86_64-python --noconfirm
	pacman -Sy mingw-w64-x86_64-python-pip --noconfirm

	#
	# Clone the repository
	#
	mkdir -p $SUPERBUILD
	cd $SUPERBUILD
	if [[ ! -d libplacebo ]]; then
	    run_cmd git clone --depth 1 --branch $LIBPLACEBO_BRANCH https://github.com/ggarra13/libplacebo.git 2> /dev/null
	fi
	
	#
	# Run configure
	#
	cd libplacebo

	export OLD_PATH="$PATH"
	
	export PATH=/mingw64/bin:/usr/bin/:/bin:$PATH

	python -m venv venv
	. venv/bin/activate
	
	python -m pip install meson

	pacman -Sy mingw-w64-x86_64-toolchain --needed --noconfirm

	
	run_cmd git submodule update --init
	
	export CC=/mingw64/bin/gcc
	export CXX=/mingw64/bin/g++

	run_cmd meson setup -Ddemos=false -Dlibdir=$INSTALLDIR/lib --prefix=$INSTALLDIR build 
	
	#
	# Compile and install the library
	#
	cd build
	ninja install

	cd -

	echo "Compiled result:"
	echo
	ls $INSTALLDIR/lib/*placebo*
	echo

	deactivate

	rm -rf venv
	
	run_cmd mv $INSTALLDIR/lib/libplacebo.dll.a $INSTALLDIR/lib/libplacebo.lib

	pacman -Rns $(pacman -Qq | grep mingw-w64) --noconfirm

	# Reinstall gettext and libiconv, libintl
	pacman -Sy mingw-w64-x86_64-gettext --noconfirm
	pacman -Sy mingw-w64-x86_64-libiconv --noconfirm

	export PATH="$OLD_PATH"
	
	cd $MRV2_ROOT
	
	unset CXX
	unset CC
	
    else
	echo "libplacebo already installed."
    fi

	
fi
