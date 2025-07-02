#!/usr/bin/bash

USE_MSVC=1
if [[ $USE_MSVC == 1 ]]; then
    pacman -R --noconfirm mingw-w64-x86_64-g++
    pacman -R --noconfirm mingw-w64-x86_64-gcc
fi
