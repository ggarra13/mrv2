#!/usr/bin/env bash

dirs="Linux-vulkan-amd64 Windows-vulkan-amd64 Darwin-vulkan-amd64 BUILD-Linux-amd64 BUILD-Windows-amd64 BUILD-Darwin-amd64"

for dir in $dirs; do
    FLTK_dir="${dir}/Release/deps/FLTK/src/FLTK"
    if [[ ! -d $FLTK_dir ]]; then
	continue
    fi
    cd $FLTK_dir
    git switch vk
    git pull
    git switch vk_merge
    git pull
    cd -
done
