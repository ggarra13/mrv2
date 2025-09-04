#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



#
#
# Wrap up script to compile FLTK, tlRender and mrv2
#
#

set +e

. $PWD/etc/build_dir.sh

rm -rf $BUILD_DIR/install/include/FL

dir=$BUILD_DIR/deps/FLTK/src/FLTK-build/
if [[ ! -d $dir ]]; then
    echo "FLTK directory"
    echo $dir
    echo "does not exist. Please run:"
    echo " $ runme.sh [sameflags]"
    exit 1
fi


cd $dir

#
#  Rebuild latest FLTK
#

cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install
if [[ $? != 0 ]]; then
    echo "COMPILATON of FLTK failed"
    cd -
    exit 1
fi

cd -

#
#  Rebuild latest pyFLTK if on Windows
#
if [[ $KERNEL == *Msys* ]]; then
    root=$PWD
    dir=$PWD/$BUILD_DIR/deps/pyFLTK/src/pyFLTK/

    echo $dir
    cd $dir

    export FLTK_HOME=$BUILD_DIR/install
    
    $root/$BUILD_DIR/install/bin/python.exe setup.py build --enable-shared --disable-forms
    $root/$BUILD_DIR/install/bin/python.exe -m pip install .
    if [[ $? != 0 ]]; then
	echo "COMPILATON of pyFLTK failed"
	cd -
	exit 1
    fi

    cd -
fi

dir=$BUILD_DIR/tlRender/etc/SuperBuild/tlRender/src/tlRender-build/
if [[ ! -d $dir ]]; then
    echo "tlRender directory:"
    echo $dir
    echo "does not exist. Please run:"
    echo " $ runme.sh [sameflags]"
    exit 1
fi

cd $dir

#
#  Rebuild tlRender
#
cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install
if [[ $? != 0 ]]; then
    echo "COMPILATON of tlRender failed"
    cd -
    exit 1
fi
cd -

dir=$BUILD_DIR/mrv2/src/mrv2-build

# Needed to to force a relink and update build info.
rm -f $dir/src/mrv2

if [[ "$CMAKE_TARGET" == "" ]]; then
    CMAKE_TARGET=install
fi

run_cmd ./runmeq.sh $CMAKE_BUILD_TYPE -t $CMAKE_TARGET
if [[ $? != 0 ]]; then
    echo "COMPILATON of mrv2 failed"
    exit 1
fi

