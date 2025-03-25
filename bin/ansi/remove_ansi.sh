#!/usr/bin/env bash

if [[ "$2" == "" ]]; then
    echo "Remove ansi codes from file.  Usage is:"
    echo ""
    echo "$0 <input> <output>"
    echo
    exit 1
fi

sed -r 's/\x1B\[([0-9]{1,3}(;[0-9]{1,3})*)?[mGK]//g' $1 > $2
