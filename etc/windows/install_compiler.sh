#!/usr/bin/bash

USE_MSVC=1
if [[ $USE_MSVC == 1 ]]; then
    pacman -Ry --noconfirm *-g++
    pacman -Ry --noconfirm *-gcc
fi
