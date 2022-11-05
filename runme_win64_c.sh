#!/bin/bash

export FLAGS=$@
export TYPE=Release

if [[ $FLAGS == "" ]]; then
    export FLAGS="-j 4"
fi

#rm -rf build_win64

mkdir -p build_win64
cd build_win64

export GLEW_ROOT=/E/code/lib/win64
export LIBINTL_ROOT=/E/code/lib/win64/libintl-win64

cmake .. -D CMAKE_BUILD_TYPE=$TYPE -G Ninja -D CMAKE_INSTALL_PREFIX=$PWD/install -D CMAKE_PREFIX_PATH=$PWD/install -D TLRENDER_PROGRAMS=OFF -D TLRENDER_EXAMPLES=FALSE -D TLRENDER_TESTS=FALSE -D TLRENDER_EXR=ON -D TLRENDER_FFMPEG=ON -D TLRENDER_QT6=OFF -D TLRENDER_QT5=FALSE -D GLEW_ROOT=${GLEW_ROOT} -D LIBINTL_ROOT=${LIBINTL_ROOT}

cmake --build . $FLAGS --config $TYPE

cd ..
