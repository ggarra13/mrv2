#!/bin/bash

export FLAGS=$@
export TYPE=Debug

if [[ $FLAGS == "" ]]; then
    export FLAGS="-j 4"
fi

rm build/install/bin/mrViewer

cd build/mrViewer/src/mrViewer2-build


make $FLAGS install

cd ../../..
