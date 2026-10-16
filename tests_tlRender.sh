#!/usr/bin/env bash

set +e

. etc/build_dir.sh

dir=$BUILD_DIR/deps/tlRender/src/tlRender-build/
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

export LD_LIBRARY_PATH=/home/gga/code/applications/mrv2/Linux-vulkan-amd64/Debug/install/lib:$LD_LIBRARY_PATH

dir=$BUILD_DIR/deps/tlRender/src/tlRender-build/tests/tltest/
if [ -d $dir ]; then
    cd $dir
    ctest --output-on-failure
    cd -
fi

dir=$BUILD_DIR/deps/tlRender/src/tlRender-build/tests/FFmpegDecodeTest/
if [ -d $dir ]; then
    cd $dir
    ctest --output-on-failure
    cd -
fi
