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

rm -rf $BUILD_DIR/install/include/tl*
rm -rf $BUILD_DIR/install/lib/tl*
rm -rf $BUILD_DIR/install/lib/libtl*

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

./runmeq.sh

. $PWD/etc/build_end.sh
