#!/usr/bin/env bash

. $PWD/etc/build_dir.sh

#
# This is needed on macOS which spits out linking errors otherwise
#
if [[ $KERNEL == *Darwin* ]]; then
    rm -f $BUILD_DIR/install/bin/mrViewer
fi

dir=$BUILD_DIR/mrViewer/src/mrViewer2-build

if [[ ! -d $dir ]]; then
    echo "mrViewer directory does not exist."
    echo "Please run:"
    echo " $ runme.sh [sameflags]"
    exit 1
fi


cd $dir

cmake --build . --config $CMAKE_BUILD_TYPE -v $FLAGS -t install

cd -

. $PWD/etc/build_end.sh
