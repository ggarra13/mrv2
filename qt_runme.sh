#!/bin/bash

export FLAGS=$@
export TYPE=Debug

if [[ $FLAGS == "" ]]; then
    export FLAGS="-j 4"
fi

rm build_qt/install/bin/tlplay

cd build_qt/tlRender/etc/SuperBuild/tlRender/src/tlRender-build

make $FLAGS install

cd ../../..
