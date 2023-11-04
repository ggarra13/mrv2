#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

# Specify the path to your .dmg file
DMG_FILE=$1
DMG_BASENAME=`basename $1`
DMG_NAME="${DMG_BASENAME%.dmg}"

# Open the .dmg file
echo "Open $DMG_FILE"
open "$DMG_FILE"

# Wait for the .dmg to be mounted
echo "Wait for /Volumes/$DMG_NAME..."
while [ ! -d "/Volumes/$DMG_NAME" ]; do
    sleep 1
done

# Optional: Display a message when the .dmg is mounted
echo "Disk image mounted. Installing mrv2..."

# Remove any previous install
echo "Removing any old installation..."
rm -rf /Applications/mrv2.app

# You can add additional commands here to automate the installation if needed
echo "Installing ${DMG_NAME}..."
cp -rf /Volumes/${DMG_NAME}/mrv2.app /Applications/

# When you're done, unmount the .dmg (optional)
hdiutil detach "/Volumes/${DMG_NAME}"

# Optional: Display a message when the .dmg is unmounted
echo "Disk image unmounted."

# Exit the script
exit 0
