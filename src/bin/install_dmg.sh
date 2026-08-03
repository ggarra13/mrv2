#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

# Specify the path to your .dmg file
DMG_FILE=$1
DMG_BASENAME=`basename $1`
DMG_NAME="${DMG_BASENAME%.dmg}"
DMG_PREFIX="${DMG_NAME%%-*}"

# Remove the quarantine attribute from DMG
xattr -d com.apple.quarantine "$DMG_FILE"

# Open the .dmg file
echo "Open $DMG_FILE"
yes 2> /dev/null | hdiutil attach -noverify "$DMG_FILE"

# Wait for the .dmg to be mounted
echo "Wait for /Volumes/$DMG_NAME..."
while [ ! -d "/Volumes/$DMG_NAME" ]; do
    sleep 1
done

# Optional: Display a message when the .dmg is mounted
echo "Disk image mounted. Installing ${DMG_PREFIX}..."

# # Remove any previous install
echo "Removing any old installation of ${DMG_PREFIX}.app ..."
rm -rf /Applications/${DMG_PREFIX}.app

# You can add additional commands here to automate the installation if needed
echo "Installing ${DMG_PREFIX}..."
cp -rf /Volumes/${DMG_NAME}/${DMG_PREFIX}.app /Applications/

if [[ -e /Volumes/${DMG_NAME}/hdr.app ]]; then
    rm -rf /Applications/hdr.app
    cp -rf /Volumes/${DMG_NAME}/hdr.app /Applications/
fi

echo "Running installation.command script to finish installation..."
open /Volumes/${DMG_NAME}/installation.command

# 1. Wait up to 15 seconds for the new Terminal window to actually start the
#    script
echo "Waiting for Terminal to launch..."
for i in {1..15}; do
    if pgrep -f "/Volumes/${DMG_NAME}/installation.command" > /dev/null; then
        # The process has started, break out of this initial waiting loop
        break
    fi
    sleep 1
done

# 2. Now wait for the script to finish and the process to disappear
echo "Waiting for installation to complete (please enter your password in the new window)..."
while pgrep -f "/Volumes/${DMG_NAME}/installation.command" > /dev/null; do
    sleep 1
done

# When you're done, unmount the .dmg (optional)
hdiutil detach "/Volumes/${DMG_NAME}"

# Optional: Display a message when the .dmg is unmounted
echo "Disk image unmounted."

# Exit the script
exit 0
