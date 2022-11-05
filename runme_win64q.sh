#!/bin/bash

export FLAGS=$@
export TYPE=Release

if [[ $FLAGS == "" ]]; then
    export FLAGS="-j 4"
fi

rm build_win64/install/bin/mrViewer.exe

export GLEW_ROOT=/E/code/lib/win64

cd build_win64/mrViewer/src/mrViewer2-build

cmake --build . $FLAGS --config $TYPE -t install

cd ../../..
