#!/usr/bin/env bash

# Exit on error
set -e

# Root directory (default: current directory)
ROOT_DIR="${1:-.}"

# Find and replace
find "$ROOT_DIR" \( -name "*.h" -o -name "*.cpp" -o -name "*.fl" \) -type f | while read -r file; do
    if grep -q 'mrvCore/mrvString.h' "$file"; then
        echo "Updating: $file"
        sed -i 's|#mrvCore/mrvString.h|#mrvOS/mrvString.h|g' "$file"
    fi
done
