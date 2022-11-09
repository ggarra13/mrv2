#!/bin/bash

. build_dir.sh

rm $BUILD_DIR/install/bin/mrViewer

cd $BUILD_DIR/mrViewer/src/mrViewer2-build

cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install

cd -

rm ~/bin/mrv2

chmod a+x $PWD/$BUILD_DIR/install/bin/mrViewer.sh
ln -s $PWD/$BUILD_DIR/install/bin/mrViewer.sh ~/bin/mrv2

