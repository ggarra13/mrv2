#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Get the auxiliary functions
#

if [[ "$BUILD_DIR" == "" ]]; then
    . etc/build_dir.sh
fi

extract_version

# Specify the paths and filenames
SIGNTOOL_PATH="signtool.exe"
PFX_FILE="${PWD}/certificates/mrv2.pfx"

mrv2_NAME=mrv2
if [[ $MRV2_BACKEND == "VK" ]]; then
    mrv2_NAME=vmrv2
fi

NSIS_INSTALLER="${PWD}/paquetes/${BUILD_DIR}/${mrv2_NAME}-v${mrv2_VERSION}-Windows-${ARCH}.exe"

AZURE_HTTP="http://timestamp.comodoca.com/authenticode"

# Define password variable
PASS="secretsalt1973!"

sign_installer() {
    run_cmd "$SIGNTOOL_PATH" sign -v -f "$PFX_FILE" -p "$PASS" -fd SHA256 -t "${AZURE_HTTP}" "$NSIS_INSTALLER"
}

#
# Call the function to sign the NSIS installer
# if I am USER-PC or on GITHUB_ACTIONS.
#
if [[ "$USER" == "User-PC" || "$GITHUB_ACTIONS" == "true" ]]; then
    if [[ -e "${NSIS_INSTALLER}" ]]; then
        echo "Installer found. Starting signing process..."
        sign_installer
    else
        echo "Error: Installer not found at ${NSIS_INSTALLER}"
    fi
fi
