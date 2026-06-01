#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

if [[ "$1" == "" ]]; then
    echo "gh_release.sh <version>"
    exit 1
fi

OUTPUT_FILE=SHA256SUMS

cd releases/beta
rm -rf $OUTPUT_FILE

files=""
for i in vulkan/*$1*; do
    files="$i $files"
done

for i in opengl/*$1*; do
    files="$i $files"
done

for file in $files; do
    echo "Adding checksum for $file..."
    sha256sum "$file" >> "$OUTPUT_FILE"
done

sed -i 's#opengl/##' $OUTPUT_FILE
sed -i 's#vulkan/##' $OUTPUT_FILE

files="${OUTPUT_FILE} ${files}"

echo $files

gh release upload $1 $files --clobber 

