#!/usr/bin/env bash

# 1. Generate a 2048-bit private key
openssl genrsa -out mrv2-selfsigned.key 2048

# 2. Create a self-signed certificate with the Code Signing extended key usage
openssl req -new -x509 \
    -key  mrv2-selfsigned.key \
    -out  mrv2-selfsigned.crt \
    -days 3650 \
    -subj "/CN=mrv2-selfsigned/O=mrv2 Project/C=ES" \
    -addext "keyUsage=critical,digitalSignature" \
    -addext "extendedKeyUsage=codeSigning"

# 3. Bundle key + cert into PKCS#12 (required for Keychain import)
#    You will be asked to set an export password — remember it for the next step
openssl pkcs12 -export \
    -legacy \
    -in    mrv2-selfsigned.crt \
    -inkey mrv2-selfsigned.key \
    -out   mrv2-selfsigned.p12 \
    -name  "mrv2-selfsigned"

# 4. Import into your login keychain
#    Enter the export password you set above when prompted
security import mrv2-selfsigned.p12 \
    -k ~/Library/Keychains/login.keychain-db \
    -T /usr/bin/codesign

#
# Certificates imported from the command line are untrusted by default. This command elevates them — macOS will show an admin password prompt:
#
security add-trusted-cert \
    -d -r trustRoot \
    -k ~/Library/Keychains/login.keychain-db \
    mrv2-selfsigned.crt
