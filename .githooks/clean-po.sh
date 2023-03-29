#!/bin/sh

filename="$PWD/$1"
sed -e '/^#: /d' "$filename"
