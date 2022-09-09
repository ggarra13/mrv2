#!/bin/bash

export FLAGS=$@
export TYPE=Debug

if [[ $FLAGS == "" ]]; then
    export FLAGS="-j 4"
    export TYPE="Release"
fi

rm -rf build

mkdir -p build
cd build


cmake .. -D CMAKE_BUILD_TYPE=$TYPE -D CMAKE_INSTALL_PREFIX=$PWD/install -D CMAKE_PREFIX_PATH=$PWD/install  -D TLRENDER_BUILD_PROGRAMS=OFF -D TLRENDER_BUILD_EXAMPLES=FALSE -D TLRENDER_BUILD_TESTS=FALSE

cmake --build . $FLAGS --config $TYPE

cd ..
