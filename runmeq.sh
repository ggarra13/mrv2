#!/bin/bash

. $PWD/aux/build_dir.sh

rm -f $BUILD_DIR/install/bin/mrViewer

cd $BUILD_DIR/mrViewer/src/mrViewer2-build

cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -v -t install

cd -

. $PWD/aux/build_end.sh
