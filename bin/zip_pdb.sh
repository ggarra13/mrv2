#!/usr/bin/env bash

. etc/build_dir.sh


# Output zip file name
ZIPFILE="mrv2_pdb_files.zip"

find "$BUILD_DIR" -type f -name "*.pdb" -exec zip output_pdb_files.zip {} +

echo "PDB files and directories zipped into $ZIPFILE"
