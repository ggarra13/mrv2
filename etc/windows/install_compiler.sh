#!/usr/bin/bash

USE_MSVC=1
if [[ $USE_MSVC == 1 ]]; then
    pacman -R --noconfirm *-g++
    pacman -R --noconfirm *-gcc
fi
