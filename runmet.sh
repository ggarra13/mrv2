#!/usr/bin/env bash

. $PWD/aux/build_dir.sh


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

. $PWD/runmeq.sh
