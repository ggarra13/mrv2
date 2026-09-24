#!/usr/bin/env bash

if [[ "$1" == "" ]]; then
    echo "$0 <file>"
fi

#ffprobe -show_frames -show_entries frame_side_data "$1"

# Read the first two frames
ffprobe -select_streams v -read_intervals "%+#2" -show_frames -show_entries frame_side_data "$1"
