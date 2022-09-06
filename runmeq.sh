#!/bin/bash

export FLAGS=$@
export TYPE=Debug

if [[ $FLAGS == "" ]]; then
    export FLAGS="-j 4"
    export TYPE="Release"
fi

mkdir -p build

cd build/mrViewer/src/mrViewer2-build

make $FLAGS install

cd ../../..
