#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/bin/bash

# Specify the path to your .dmg file
DMG_FILE=$1
DMG_NAME=`echo $1 | sed -e 's/.dmg//'` 

# Open the .dmg file
open "$DMG_FILE"

# Wait for the .dmg to be mounted
while [ ! -d "/Volumes/${DMG_NAME}" ]; do
    sleep 1
done

# Optional: Display a message when the .dmg is mounted
echo "Disk image mounted. Installing mrv2..."

# Remove any previous install
echo "Removing any old installation..."
rm -rf /Applications/mrv2.app

# You can add additional commands here to automate the installation if needed
echo "Installing ${DMG_NAME}..."
cp -f /Volumes/${DMG_NAME}/mrv2.app /Applications

# When you're done, unmount the .dmg (optional)
hdiutil detach "/Volumes/${DMG_NAME}"

# Optional: Display a message when the .dmg is unmounted
echo "Disk image unmounted."

# Exit the script
exit 0
