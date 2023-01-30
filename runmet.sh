#!/usr/bin/env bash

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

#
# This is needed on macOS which spits out linking errors otherwise
#
rm -f $BUILD_DIR/install/bin/mrViewer

dir=$BUILD_DIR/mrViewer/src/mrViewer2-build

if [[ ! -d $dir ]]; then
    echo "mrViewer directory does not exist."
    echo "Please run:"
    echo " $ runme.sh [sameflags]"
    exit 1
fi

cd $dir

cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -v -t install

cd -

. $PWD/etc/build_end.sh
