#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



. $PWD/etc/build_dir.sh


dir=$BUILD_DIR/tlRender/etc/SuperBuild/tlRender/src/tlRender-build/
if [[ ! -d $dir ]]; then
    echo "tlRender directory does not exist. Please run:"
    echo " $ runme.sh [sameflags]"
    exit 1
fi

cd $dir

cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install

cd -

dir=$BUILD_DIR/FLTK-prefix/src/FLTK-build/
if [[ ! -d $dir ]]; then
    echo "FLTK directory does not exist. Please run:"
    echo " $ runme.sh [sameflags]"
    exit 1
fi

cd $dir

cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install

cd -

dir=$BUILD_DIR/mrv2/src/mrv2-build

rm -f $dir/src/mrv2*

if [[ $CMAKE_TARGET == doc* || $CMAKE_TARGET == "install" ||
	  $CMAKE_TARGET == "package" ]]; then
    #
    # First, generate the translations and install them
    #
    cd $dir
    cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t mo
    cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install
    cd -
fi

cd $dir

cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t $CMAKE_TARGET

cd -

. $PWD/etc/build_end.sh
