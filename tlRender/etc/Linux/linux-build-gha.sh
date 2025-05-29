#!/bin/bash

set -x

BUILD_TYPE=$1

# Update packages
sudo apt-get update

# Install lcov
if [[ $TLRENDER_GCOV = "ON" ]]
then
    sudo apt-get install lcov
fi

# Install OpenGL support
sudo apt-get install libglu1-mesa-dev mesa-common-dev mesa-utils xvfb
xvfb-run glxinfo

if [[ $TLRENDER_X11 = "ON" ]]
then
    sudo apt-get install xorg-dev
fi

# Install Wayland support
if [[ $TLRENDER_WAYLAND = "ON" ]]
then
    sudo apt-get install libwayland-dev wayland-protocols libdbus-1-dev
    sudo apt-get install libxkbcommon-dev libegl-dev libgtk-3-dev
fi

# Install ALSA and PulseAudio support
if [[ $TLRENDER_AUDIO = "ON" ]]
then
    sudo apt-get install libasound2-dev
    sudo apt-get install libpulse-dev
fi

# Install Python support
if [[ $TLRENDER_PYTHON = "ON" ]]
then
    sudo apt-get install python3.8-dev
fi

# Install Qt support
if [[ $TLRENDER_QT5 = "ON" ]]
then
    sudo apt-get install qtdeclarative5-dev libqt5quick5 qtbase5-dev libqt5svg5-dev qtchooser qt5-qmake qtbase5-dev-tools
fi

# Build tlRender
mkdir build
cd build
cmake ../etc/SuperBuild \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_INSTALL_PREFIX=$PWD/install \
    -DCMAKE_PREFIX_PATH=$PWD/install \
    -DCMAKE_CXX_STANDARD=$CMAKE_CXX_STANDARD \
    -DTLRENDER_PYTHON=$TLRENDER_PYTHON \
    -DTLRENDER_GLFW=$TLRENDER_GLFW \
    -DTLRENDER_NET=$TLRENDER_NET \
    -DTLRENDER_OCIO=$TLRENDER_OCIO \
    -DTLRENDER_AUDIO=$TLRENDER_AUDIO \
    -DTLRENDER_JPEG=$TLRENDER_JPEG \
    -DTLRENDER_TIFF=$TLRENDER_TIFF \
    -DTLRENDER_STB=$TLRENDER_STB \
    -DTLRENDER_PNG=$TLRENDER_PNG \
    -DTLRENDER_EXR=$TLRENDER_EXR \
    -DTLRENDER_FFMPEG=$TLRENDER_FFMPEG \
    -DTLRENDER_ASAN=$TLRENDER_ASAN \
    -DTLRENDER_USD=$TLRENDER_USD \
    -DTLRENDER_QT5=$TLRENDER_QT5 \
    -DTLRENDER_PROGRAMS=$TLRENDER_PROGRAMS \
    -DTLRENDER_EXAMPLES=$TLRENDER_EXAMPLES \
    -DTLRENDER_TESTS=$TLRENDER_TESTS \
    -DTLRENDER_GCOV=$TLRENDER_GCOV
cmake --build . -j $(nproc) --config $BUILD_TYPE
