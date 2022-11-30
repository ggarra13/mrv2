#!/usr/bin/env bash

. $PWD/aux/build_dir.sh

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

. $PWD/aux/build_end.sh
