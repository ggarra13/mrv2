#!/bin/bash

. build_dir.sh

cd $BUILD_DIR/tlRender/etc/SuperBuild/tlRender/src/tlRender-build/

cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install

cd -

cd $BUILD_DIR/FLTK-prefix/src/FLTK-build/

cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install

cd -

rm $BUILD_DIR/install/bin/mrViewer

cd $BUILD_DIR/mrViewer/src/mrViewer2-build

cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install

cd -

. build_end.sh
