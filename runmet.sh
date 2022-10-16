#!/bin/bash

export FLAGS=$@
export TYPE=Debug

if [[ $FLAGS == "" ]]; then
    export FLAGS="-j 4"
fi


cd build/tlRender/etc/SuperBuild/tlRender/src/tlRender-build/

cmake --build . $FLAGS --config $TYPE -t install

cd -

cd build/FLTK-prefix/src/FLTK-build/

cmake --build . $FLAGS --config $TYPE -t install

cd -

rm build/install/bin/mrViewer

cd build/mrViewer/src/mrViewer2-build

cmake --build . $FLAGS --config $TYPE -t install

cd ../../..
