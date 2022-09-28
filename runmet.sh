#!/bin/bash

export FLAGS=$@
export TYPE=Debug

if [[ $FLAGS == "" ]]; then
    export FLAGS="-j 4"
fi


cd build/tlRender/etc/SuperBuild/tlRender/src/tlRender-build/

make $FLAGS install

cd -

cd build/FLTK-prefix/src/FLTK-build/

make $FLAGS install

cd -

rm build/install/bin/mrViewer

cd build/mrViewer/src/mrViewer2-build


make $FLAGS install

cd ../../..
