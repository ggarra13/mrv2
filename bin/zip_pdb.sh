#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This Windows script collects all .pdb files of a Debug or RelWithDebInfo
# into a zipfile.
#



. etc/build_dir.sh


# Output zip file name
ZIPFILE="mrv2_pdb_files.zip"

find "$BUILD_DIR" -type f -name "*.pdb" -exec zip $ZIPFILE {} +

echo "PDB files from $BUILD_DIR zipped into $ZIPFILE"
