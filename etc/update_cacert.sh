#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

update_cacert() {
    # Download cacert from curl's (Mozilla reputable source).
    local cacert_url="https://curl.se/ca/cacert.pem"
    local hash_url="https://curl.se/ca/cacert.pem.sha256"

    # Local install location
    local dest="${1:-src/certs/cacert.pem}"

    echo "cacert: checking for updates..."

    # Fetch the SHA-256 sidecar
    local expected
    expected=$(curl -fsSL --proto '=https' --tlsv1.2 "$hash_url" | awk '{print $1}')
    if [[ -z "$expected" ]]; then
        echo "cacert: WARNING — could not fetch upstream hash; keeping existing bundle." >&2
        return 1
    fi

    # Compare against the local file (if it exists)
    if [[ -f "$dest" ]]; then
        local actual
        actual=$(sha256sum "$dest" | awk '{print $1}')
        if [[ "$actual" == "$expected" ]]; then
            echo "cacert: already up to date (SHA256=${expected})"
            return 0
        fi
    fi

    # Download the new bundle into a temp file first
    local tmp
    tmp=$(mktemp "${dest}.XXXXXX")
    rm -f $$tmp

    echo "cacert: downloading updated bundle..."
    if ! curl -fsSL --proto '=https' --tlsv1.2 -o "$tmp" "$cacert_url"; then
        echo "cacert: ERROR — download failed." >&2
        return 1
    fi

    # Verify the downloaded file matches the sidecar hash
    local downloaded_hash
    downloaded_hash=$(sha256sum "$tmp" | awk '{print $1}')
    if [[ "$downloaded_hash" != "$expected" ]]; then
        echo "cacert: ERROR — hash mismatch! Expected ${expected}, got ${downloaded_hash}. Aborting." >&2
        return 1
    fi

    # Atomically replace the destination
    chmod 644 "$tmp"
    mv "$tmp" "$dest"
    echo "cacert: updated successfully $dest (SHA256=${expected})"
}
