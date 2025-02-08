#!/usr/bin/env bash


# Import auxiliary functions
. etc/build_dir.sh

extract_version

# Script to create a self-signed .p12 certificate

# Configuration
OUTPUT_P12="${PWD}/certificates/mrv2.p12"
CERT_NAME="mrv2"                # Common name for the certificate
DAYS=3650                       # Validity period in days (10 years)
CERT_PASSWORD="mrv2" 

# Generate a private key
openssl genrsa -out key.pem 2048

# Generate a certificate signing request (CSR)
openssl req -new -key key.pem -out csr.pem -subj "/CN=${CERT_NAME}"

# Generate the self-signed certificate
openssl x509 -req -days ${DAYS} -in csr.pem -signkey key.pem -out cert.pem

# Combine the key and certificate into a .p12 file
openssl pkcs12 -export -legacy -inkey key.pem -in cert.pem -out ${OUTPUT_P12} -passout pass:${CERT_PASSWORD}

# Clean up intermediate files
rm key.pem csr.pem cert.pem

echo "Self-signed .p12 certificate created: ${OUTPUT_P12}"
echo "Password: ${CERT_PASSWORD}"

# Keychain
if [[ "$GITHUB_ACTIONS" != "" ]]; then
    KEYCHAIN_PWD=""
else
    KEYCHAIN_PWD="zxcasd"
fi

# Paths and filenames
DMG_INSTALLER="${PWD}/packages/mrv2-v${mrv2_VERSION}-${KERNEL}-${ARCH}.dmg"

# Timestamp server for macOS (optional but recommended)
TIMESTAMP_SERVER="http://timestamp.apple.com/ts01"

# Function to import the certificate and sign the DMG

sign_installer() {
    # Unlock keychain
    echo "Unlocking keychain..."
    security unlock-keychain -p "$KEYCHAIN_PWD" ~/Library/Keychains/login.keychain
    if [[ $? -ne 0 ]]; then
        echo "Error unlocking keychain."
        return 1  # Indicate failure
    else
        echo "Unlocked keychain"
    fi

    # Import the certificate temporarily
    echo "Importing certificate..."
    security import "$OUTPUT_P12" -k ~/Library/Keychains/login.keychain -P "$CERT_PASSWORD" -T /usr/bin/codesign
    if [[ $? -ne 0 ]]; then
        echo "Error importing certificate. Check password and certificate file."
        return 1  # Indicate failure
    else
        echo "Imported certificate"
    fi

    # Verify that the certificate exist AND is valid
    echo "Verifying certificate import and validity..."
    security find-identity -v -p codesigning ~/Library/Keychains/login.keychain
    if [[ $? -ne 0 ]]; then
	echo "Error finding any code signing identities after import. Import likely failed."
	return 1
    else
	echo "Code-signing identities found"
    fi

   # Get the certificate identity - More Robust Approach
    echo "Finding signing identity..."
    SIGN_ID=$(security find-certificate -c "mrv2" -p codesigning ~/Library/Keychains/login.keychain | awk '/-----BEGIN CERTIFICATE-----/{print; exit}')

    if [[ -z "$SIGN_ID" ]]; then
        echo "Error finding signing identity. Verify certificate name matches what's in the .p12."
        #Clean up the keychains with -c "mrv2"
	security delete-certificate -c "mrv2" ~/Library/Keychains/login.keychain
        return 1
    else
	echo "Certificate identity found"
    fi

    # Sign the DMG
    echo "Signing DMG..."
    codesign --sign "$SIGN_ID" --timestamp --verbose=4 "$DMG_INSTALLER"
    if [[ $? -ne 0 ]]; then
        echo "Error signing DMG."
        security delete-certificate -c "mrv2" ~/Library/Keychains/login.keychain
        return 1
    else
        echo "Signed .dmg"
    fi

    # Clean up: remove certificate (optional)
    echo "Deleting temporary certificate..."
    security delete-certificate -c "mrv2" ~/Library/Keychains/login.keychain
    if [[ $? -ne 0 ]]; then
        echo "Error deleting certificate. This may not be a critical error."
        security delete-certificate -c "mrv2" ~/Library/Keychains/login.keychain #Attempt force removal
    else
        echo "Deleted temporary certificate"
    fi

}


sign_installer
