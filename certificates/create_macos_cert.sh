#!/usr/bin/env bash

rm mrv2.csr mrv2.crt mrv2.p12

# Create a private key
openssl genrsa -out mrv2.key 2048

# Generate a certificate signing request (CSR)
openssl req -new -key mrv2.key -out mrv2.csr -subj "/CN=mrv2 Developer/O=mrv2 Project/C=US"

# Create the self-signed certificate (valid for 10 years)
openssl x509 -req -days 3650 -in mrv2.csr -signkey mrv2.key -out mrv2.crt

openssl pkcs12 -export -out mrv2.p12 -inkey mrv2.key -in mrv2.crt -name "mrv2" -password pass:mrv2_13091973
