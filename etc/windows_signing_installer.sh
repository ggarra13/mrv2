#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/bin/bash

#
# Get the auxiliary functions
#
. etc/functions.sh

extract_version

# Specify the paths and filenames
SIGNTOOL_PATH="signtool.exe"
PFX_FILE="${PWD}/certificates/mrv2.pfx"
NSIS_INSTALLER="${PWD}/packages/mrv2-v${mrv2_VERSION}-Windows-amd64.exe"

AZURE_HTTP="http://timestamp.comodoca.com/authenticode"

sign_installer() {
    "$SIGNTOOL_PATH" sign -v -f "$PFX_FILE" -t "${AZURE_HTTP}" "$NSIS_INSTALLER"
    "$SIGNTOOL_PATH" verify -v -pa "$NSIS_INSTALLER"
}

# Call the function to sign the NSIS installer
sign_installer
