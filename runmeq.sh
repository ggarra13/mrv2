#!/bin/bash

. build_dir.sh


cd $BUILD_DIR/mrViewer/src/mrViewer2-build

cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install

cd -

. build_end.sh
