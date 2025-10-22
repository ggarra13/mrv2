#!/usr/bin/env bash 

for pofile in src/po/mrv2/*.po; do
    echo $pofile
    msgfmt -v $pofile -o /dev/null
done
