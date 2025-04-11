#!/bin/sh

for f in $1/*.svg
do
    for dpi in 96 192
    do
        base=$(basename -s .svg $f)
        png=${base}_${dpi}dpi.png
        inkscape $f -o $png -d $dpi
        h=$2/${base}_${dpi}.h
        bin/tlresource/tlresource $png $h ${base}_${dpi}_png
    done
done

