#!/usr/bin/env bash

# Exit on error
set -e

# Root directory (default: current directory)
ROOT_DIR="${1:-.}"

orig=mrvFLTK/mrvSaveOptions.h
new=mrvOptions/mrvSaveOptions.h

# Find and replace
find "$ROOT_DIR" \( -name "*.h" -o -name "*.cpp" -o -name "*.fl" \) -type f | while read -r file; do
    if grep -q "$orig" "$file"; then
        echo "Updating: $file"
        sed -i "s|$orig|$new|g" "$file"
    fi
done
