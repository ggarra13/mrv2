#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/usr/bin/env bash


# Script to create a self-signed .p12 certificate

# Configuration
CERT_NAME="MySelfSignedCert"    # Common name for the certificate
DAYS=3650                       # Validity period in days (10 years)
OUTPUT_P12="mrv2.p12"           # Output .p12 file name
PASSWORD="mrv2"                 # Password for the .p12 file

# Generate a private key
openssl genrsa -out key.pem 2048

# Generate a certificate signing request (CSR)
openssl req -new -key key.pem -out csr.pem -subj "/CN=${CERT_NAME}"

# Generate the self-signed certificate
openssl x509 -req -days ${DAYS} -in csr.pem -signkey key.pem -out cert.pem

# Combine the key and certificate into a .p12 file
openssl pkcs12 -export -legacy -inkey key.pem -in cert.pem -out ${OUTPUT_P12} -passout pass:${PASSWORD}

# Clean up intermediate files
rm key.pem csr.pem cert.pem

echo "Self-signed .p12 certificate created: ${OUTPUT_P12}"
echo "Password: ${PASSWORD}"
