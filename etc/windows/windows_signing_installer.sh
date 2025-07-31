#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Get the auxiliary functions
#
. etc/functions.sh

extract_version

# Specify the paths and filenames
SIGNTOOL_PATH="signtool.exe"
PFX_FILE="${PWD}/certificates/mrv2.pfx"

mrv2_NAME=mrv2
if [[ $MRV2_BACKEND == "VK" ]]; then
    mrv2_NAME=vmrv2
fi

NSIS_INSTALLER="${PWD}/packages/${mrv2_NAME}-v${mrv2_VERSION}-Windows-amd64.exe"

AZURE_HTTP="http://timestamp.comodoca.com/authenticode"

sign_installer() {
    run_cmd "$SIGNTOOL_PATH" sign -v -f "$PFX_FILE" -fd SHA256 -t "${AZURE_HTTP}" "$NSIS_INSTALLER"
}

# Call the function to sign the NSIS installer
if [[ "$USER" == "User-PC" && -e "${NSIS_INSTALLER}" ]]; then
    sign_installer
fi
