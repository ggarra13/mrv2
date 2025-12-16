#!/usr/bin/env bash

if [[ "$1" == "" ]]; then
    echo "gh_release.sh <version>"
    exit 1
fi

files=""
for i in releases/beta/vulkan/*$1*; do
    files="$i $files"
done

for i in releases/beta/opengl/*$1*; do
    files="$i $files"
done

echo $files

gh release upload $1 $files --clobber 
