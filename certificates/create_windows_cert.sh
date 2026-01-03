#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/bin/bash

# Configuration - Match this exactly to your CMake CPACK_PACKAGE_VENDOR
VENDOR_NAME="Gonzalo Garramuno (ggarra13@gmail.com)"
OUTPUT_FILE="mrv2.pfx"
PASS="secretsalt1973!"

echo "Removing old files..."
rm -f mrv2.key mrv2.crt "$OUTPUT_FILE"

echo "1. Generating Private Key and Certificate..."
# We create a self-signed cert in one step. 
# -nodes ensures the intermediate .key file isn't encrypted (we'll encrypt the PFX instead).
openssl req -x509 -newkey rsa:4096 -keyout mrv2.key -out mrv2.crt -sha256 -days 1095 -nodes \
  -subj "/CN=$VENDOR_NAME"

echo "2. Converting to PFX file..."
# This bundles the key and cert into the PFX format Windows requires.
openssl pkcs12 -export -out "$OUTPUT_FILE" -inkey mrv2.key -in mrv2.crt \
  -name "mrv2 Code Signing" -passout pass:"$PASS"

echo "3. Cleaning up temporary files..."
rm mrv2.key mrv2.crt

echo "Done! Generated $OUTPUT_FILE with Vendor: $VENDOR_NAME"
