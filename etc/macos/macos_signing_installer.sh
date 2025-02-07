#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

# Import auxiliary functions
. etc/build_dir.sh

extract_version

# Paths and filenames
CERTIFICATE="${PWD}/certificates/mrv2.p12"
CERT_PASSWORD="mrv2_13091973"  # Or get this from an environment variable
DMG_INSTALLER="${PWD}/packages/mrv2-v${mrv2_VERSION}-${KERNEL}-${ARCH}.dmg"

# Timestamp server for macOS (optional but recommended)
TIMESTAMP_SERVER="http://timestamp.apple.com/ts01"

# Function to import the certificate and sign the DMG
sign_installer() {
    # Import the certificate temporarily
    security import "$CERTIFICATE" -k ~/Library/Keychains/login.keychain-db -P "$CERT_PASSWORD" -T /usr/bin/codesign

    # Get the certificate identity
    SIGN_ID=$(security find-identity -p codesigning -v | awk -F\" '/"[^"]+"/ {print $2}' | head -n1)

    # Sign the DMG
    codesign --sign "$SIGN_ID" --timestamp --verbose=4 "$DMG_INSTALLER"

    # Clean up: remove certificate (optional)
    security delete-certificate -c "mrv2" ~/Library/Keychains/login.keychain-db
}

sign_installer
