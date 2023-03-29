#!/bin/bash

filename="$PWD/$1"
awk '/^msgid/ { printf "%s%s\n", $0, NR == 1 ? "": "\n" } /^msgstr/ { print }' "$filename"
