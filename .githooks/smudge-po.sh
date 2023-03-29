#!/bin/bash

rootdir=$(git rev-parse --show-toplevel)
filename=$(git ls-files --full-name -- "${1}")
fullpath="${rootdir}/${filename}"
awk '/^msgid/ { printf "%s%s\n", $0, NR == 1 ? "": "\n" } /^msgstr/ { print }' "${fullpath}"
