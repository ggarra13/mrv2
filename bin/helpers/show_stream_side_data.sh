#!/usr/bin/env bash

if [[ "$1" == "" ]]; then
    echo "$0 <file>"
fi

ffprobe -show_streams -show_entries stream_side_data "$1"
