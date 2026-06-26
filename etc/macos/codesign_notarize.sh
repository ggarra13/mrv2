#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2 — macOS Code Sign & Notarize Script
# Copyright Contributors to the mrv2 Project. All rights reserved.
#
# Usage: ./codesign_notarize.sh [OPTIONS] COMMAND
#
# Place this script at the root of the mrv2 repository (alongside CMakeLists.txt).
# Run `./codesign_notarize.sh --help` for full documentation.

set -eo pipefail

. etc/functions.sh

get_kernel
extract_version


# ─────────────────────────────────────────────────────────────────────────────
#  CONFIGURATION
#  Override any of these via environment variables or CLI flags.
# ─────────────────────────────────────────────────────────────────────────────

# Developer ID Application certificate — exactly as it appears in Keychain Access.
# Example: "Developer ID Application: Gonzalo Garramuño (XXXXXXXXXX)"
DEVELOPER_ID="${DEVELOPER_ID:-}"

# Apple ID (email) associated with your developer account.
APPLE_ID="${APPLE_ID:-ggarra13@gmail.com}"

# Apple Team ID (10-character string, visible at developer.apple.com).
TEAM_ID="${TEAM_ID:-XXXXXXXXXX}"

# App-specific password created at appleid.apple.com > Sign-In and Security.
# Pass the literal value OR use the @keychain: prefix to avoid shell history exposure:
#   xcrun notarytool store-credentials  ← run once; then set:
#   APP_PASSWORD="@keychain:mrv2-notarytool"
APP_PASSWORD="${APP_PASSWORD:-@keychain:mrv2-notarytool}"

# Keychain profile name used with xcrun notarytool store-credentials (run once).
NOTARYTOOL_PROFILE="${NOTARYTOOL_PROFILE:-mrv2-notarytool}"

# CPack / build output directory that contains the .app bundles and the DMG.

ROOT_DIR=BUILD-${KERNEL}-${ARCH}
mrv2_NAME="mrv2"

# Names of the two app bundles produced by CPack.
MRV2_APP="${MRV2_APP:-mrv2.app}"
VMRV2_APP="${VMRV2_APP:-vmrv2.app}"
HDR_APP="${HDR_APP:-hdr.app}"         # Set HDR_APP="" to skip the hdr bundle.


# Entitlements file for the main executables.
# A default file is generated automatically if this path does not exist.
ENTITLEMENTS="${ENTITLEMENTS:-$(pwd)/etc/macOS/mrv2.entitlements}"

# Minimum macOS version to target in the generated entitlements (informational).
MACOS_MIN_VERSION="${MACOS_MIN_VERSION:-12.0}"

# ─────────────────────────────────────────────────────────────────────────────
#  COLOURS
# ─────────────────────────────────────────────────────────────────────────────
_R='\033[0;31m' _G='\033[0;32m' _Y='\033[1;33m' _B='\033[0;34m' _C='\033[0;36m' _N='\033[0m'
info()    { echo -e "${_B}[INFO]${_N}  $*"; }
ok()      { echo -e "${_G}[ OK ]${_N}  $*"; }
warn()    { echo -e "${_Y}[WARN]${_N}  $*"; }
step()    { echo -e "\n${_C}════════════════════════════════════════${_N}"; echo -e "${_C} $*${_N}"; echo -e "${_C}════════════════════════════════════════${_N}"; }
die()     { echo -e "${_R}[ERR ]${_N}  $*" >&2; exit 1; }

# ─────────────────────────────────────────────────────────────────────────────
#  HELPERS
# ─────────────────────────────────────────────────────────────────────────────

require_cmd() {
    command -v "$1" &>/dev/null || die "Required command not found: $1 — install Xcode Command Line Tools."
}

check_dependencies() {
    step "Checking dependencies"
    for cmd in codesign xcrun hdiutil ditto file find; do
        require_cmd "$cmd"
    done
    ok "All required tools found."
}

check_certificate() {
    if [[ "${DEVELOPER_ID}" == "" ]]; then
	echo "DEVELOPER_ID not set. Cannot verify signing certificate"
	exit 0
    fi
    step "Verifying signing certificate"
    security find-identity -v -p codesigning \
        | grep -F "${DEVELOPER_ID}" &>/dev/null \
        || die "Certificate not found in Keychain:\n  '${DEVELOPER_ID}'\nImport your Developer ID certificate and try again."
    ok "Certificate found: ${DEVELOPER_ID}"
}

# ─────────────────────────────────────────────────────────────────────────────
#  ENTITLEMENTS
# ─────────────────────────────────────────────────────────────────────────────

generate_entitlements() {
    step "Entitlements"

    if [[ -f "${ENTITLEMENTS}" ]]; then
        ok "Using existing entitlements: ${ENTITLEMENTS}"
        return 0
    fi

    info "Generating default entitlements → ${ENTITLEMENTS}"
    mkdir -p "$(dirname "${ENTITLEMENTS}")"

    cat > "${ENTITLEMENTS}" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
    "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>

    <!--
        Hardened Runtime — required for notarization.
        https://developer.apple.com/documentation/security/hardened_runtime
    -->

    <!--
        mrv2 ships its own dylibs (OpenColorIO, OpenEXR, FFmpeg, etc.)
        that are NOT signed by Apple. Disable library validation so
        codesign/Gatekeeper does not reject them at launch.
    -->
    <key>com.apple.security.cs.disable-library-validation</key>
    <true/>

    <!--
        Python embeds a JIT and loads .pyc byte-code into executable pages.
        Required when BUILD_PYTHON=ON / MRV2_PYBIND11=ON.
    -->
    <key>com.apple.security.cs.allow-unsigned-executable-memory</key>
    <true/>

    <!--
        MRV2_NETWORK=ON: allow outbound connections (media streaming,
        RV sync protocol, NDI discovery, etc.).
    -->
    <key>com.apple.security.network.client</key>
    <true/>

    <!--
        Media viewer: read/write user-selected files (open, save, export).
    -->
    <key>com.apple.security.files.user-selected.read-write</key>
    <true/>

    <!--
        Allow reading from the user's Movies, Music, and Pictures folders
        when the user opens a file from those locations.
    -->
    <key>com.apple.security.assets.movies.read-only</key>
    <true/>
    <key>com.apple.security.assets.music.read-only</key>
    <true/>
    <key>com.apple.security.assets.pictures.read-only</key>
    <true/>

    <!--
        Uncomment if mrv2 captures audio (e.g. live-input monitoring).
    -->
    <!-- <key>com.apple.security.device.audio-input</key><true/> -->

    <!--
        Uncomment if mrv2 accesses a camera feed.
    -->
    <!-- <key>com.apple.security.device.camera</key><true/> -->

</dict>
</plist>
PLIST

    ok "Entitlements written to ${ENTITLEMENTS}"
}

# ─────────────────────────────────────────────────────────────────────────────
#  LOW-LEVEL SIGNING HELPERS
# ─────────────────────────────────────────────────────────────────────────────

# Sign a Mach-O binary or framework with the hardened runtime.
_sign_macho() {
    local target="$1"
    local ents="${2:-}"

    local args=(
        --force
        --options runtime
        --sign "${DEVELOPER_ID}"
        --timestamp
    )
    [[ -n "${ents}" && -f "${ents}" ]] && args+=(--entitlements "${ents}")

    codesign "${args[@]}" "${target}" 2>&1 | grep -v "^$" || true
}

# Sign a plain file (shell script, resource) — no hardened-runtime flag needed.
_sign_plain() {
    codesign \
        --force \
        --sign "${DEVELOPER_ID}" \
        --timestamp \
        "${1}" 2>&1 | grep -v "^$" || true
}

# Return 0 if the path contains a Mach-O object.
_is_macho() { file "$1" | grep -qE "Mach-O|universal binary"; }

# ─────────────────────────────────────────────────────────────────────────────
#  SIGN A SINGLE APP BUNDLE  (inside-out: dylibs → frameworks → exe → bundle)
# ─────────────────────────────────────────────────────────────────────────────

sign_bundle() {
    local bundle="$1"
    [[ -d "${bundle}" ]] || { warn "Bundle not found, skipping: ${bundle}"; return 0; }

    step "Signing bundle: $(basename "${bundle}")"

    # ── 1. Individual .dylib files ────────────────────────────────────────────
    info "Signing .dylib files…"
    while IFS= read -r -d '' lib; do
        info "  dylib: ${lib#"${bundle}/"}"
        _sign_macho "${lib}"
    done < <(find "${bundle}" -name "*.dylib" -print0)

    # ── 2. Native extension modules (.so) — Python, Qt plugins, etc. ─────────
    info "Signing .so extension modules…"
    while IFS= read -r -d '' so; do
        info "  so:    ${so#"${bundle}/"}"
        _sign_macho "${so}"
    done < <(find "${bundle}" -name "*.so" -print0)

    # ── 3. Frameworks (sign the whole framework bundle, not just the binary) ──
    # Process deepest first to avoid signing a framework that contains
    # an already-signed sub-framework.
    info "Signing frameworks…"
    while IFS= read -r -d '' fw; do
        info "  fw:    ${fw#"${bundle}/"}"
        _sign_macho "${fw}"
    done < <(find "${bundle}" -name "*.framework" -print0 | sort -rz)

    # ── 4. Python stdlib and extension binaries ───────────────────────────────
    #    Python .pyc / .py files are not signed; only compiled .so modules
    #    (already handled above) and the Python interpreter itself matter.
    info "Signing Python interpreter (if present)…"
    while IFS= read -r -d '' py_bin; do
        if _is_macho "${py_bin}"; then
            info "  py:    ${py_bin#"${bundle}/"}"
            _sign_macho "${py_bin}" "${ENTITLEMENTS}"
        fi
    done < <(find "${bundle}" -name "python*" -type f -print0)

    # ── 5. Every other Mach-O binary in Contents/MacOS and Contents/bin ───────
    info "Signing executables…"
    while IFS= read -r -d '' exe; do
        # Skip symlinks (they don't need signing) and already-handled dirs.
        [[ -L "${exe}" ]] && continue
        if _is_macho "${exe}"; then
            info "  exe:   ${exe#"${bundle}/"}"
            _sign_macho "${exe}" "${ENTITLEMENTS}"
        fi
    done < <(find "${bundle}/Contents/MacOS" "${bundle}/Contents/bin" \
                  -maxdepth 3 -type f -print0 2>/dev/null)

    # ── 6. Shell-script launchers (mrv2.sh / launcher.sh / hdr.sh) ───────────
    #    Shell scripts must also be signed as part of the hardened runtime.
    info "Signing shell-script launchers…"
    while IFS= read -r -d '' sh; do
        info "  sh:    ${sh#"${bundle}/"}"
        _sign_plain "${sh}"
    done < <(find "${bundle}/Contents/MacOS" -name "*.sh" -type f -print0 2>/dev/null)

    # ── 7. Sign the whole app bundle last ─────────────────────────────────────
    info "Signing top-level bundle…"
    _sign_macho "${bundle}" "${ENTITLEMENTS}"

    # ── Verify ────────────────────────────────────────────────────────────────
    info "Verifying code signature…"
    codesign --verify --deep --strict --verbose=2 "${bundle}" \
        || die "Signature verification failed for ${bundle}."
    ok "Signature verified: $(basename "${bundle}")"

    info "Running Gatekeeper pre-check…"
    if spctl --assess --type exec --verbose "${bundle}" 2>&1; then
        ok "Gatekeeper assessment passed."
    else
        warn "Gatekeeper pre-check failed — this is expected before notarization."
    fi
}

# ─────────────────────────────────────────────────────────────────────────────
#  SIGN ALL APP BUNDLES
# ─────────────────────────────────────────────────────────────────────────────

sign_all_bundles() {
    if [[ -n "${MRV2_APP:-}" ]]; then
	sign_bundle "${PACK_DIR}/${MRV2_APP}"
    fi
    if [[ -n "${VMRV2_APP:-}" ]]; then
	sign_bundle "${PACK_DIR}/${VMRV2_APP}"
    fi
    if [[ -n "${HDR_APP:-}" ]]; then
        sign_bundle "${PACK_DIR}/${HDR_APP}"
    fi
}

# ─────────────────────────────────────────────────────────────────────────────
#  DMG CREATION  (when you don't want to use CPack's DMG)
# ─────────────────────────────────────────────────────────────────────────────

create_dmg() {
    step "Creating DMG: ${DMG_NAME}"

    runmeq.sh -t package ${VK_ARG}
    
    # local src_apps=()
    # [[ -d "${PACK_DIR}/${MRV2_APP}" ]] && src_apps+=("${PACK_DIR}/${MRV2_APP}")
    # [[ -n "${HDR_APP:-}" && -d "${PACK_DIR}/${HDR_APP}" ]] \
    #     && src_apps+=("${PACK_DIR}/${HDR_APP}")
    # [[ ${#src_apps[@]} -gt 0 ]] || die "No app bundles found in ${PACK_DIR}."

    # local staging_dir
    # staging_dir="$(mktemp -d)"
    # trap 'rm -rf "${staging_dir}"' RETURN

    # # Copy apps into a staging folder.
    # for app in "${src_apps[@]}"; do
    #     ditto "${app}" "${staging_dir}/$(basename "${app}")"
    # done

    # # Create a symlink to /Applications for drag-install UX.
    # ln -s /Applications "${staging_dir}/Applications"

    # local tmp_dmg="${PACK_DIR}/tmp_$$.dmg"
    # local final_dmg="${PACK_DIR}/${DMG_NAME}"

    # rm -f "${tmp_dmg}" "${final_dmg}"

    # hdiutil create \
    #     -volname "mrv2 Installer" \
    #     -srcfolder "${staging_dir}" \
    #     -ov -format UDRW \
    #     "${tmp_dmg}"

    # hdiutil convert "${tmp_dmg}" \
    #     -format UDZO \
    #     -imagekey zlib-level=9 \
    #     -o "${final_dmg}"

    # rm -f "${tmp_dmg}"
    # ok "DMG created: ${final_dmg}"
}

# ─────────────────────────────────────────────────────────────────────────────
#  SIGN DMG
# ─────────────────────────────────────────────────────────────────────────────

sign_dmg() {
    local dmg="${PACKAGE_DIRECTORY}/${DMG_NAME}"
    step "Signing DMG: ${dmg}"
    [[ -f "${dmg}" ]] || die "DMG not found: ${dmg}\nRun 'create-dmg' or point BUILD_DIR at the CPack output directory."

    codesign \
        --force \
        --sign "${DEVELOPER_ID}" \
        --timestamp \
        --verbose=2 \
        "${dmg}"

    ok "DMG signed."
}

# ─────────────────────────────────────────────────────────────────────────────
#  NOTARIZE
# ─────────────────────────────────────────────────────────────────────────────

notarize() {
    local target="${1:-${PACKAGE_DIRECTORY}/${DMG_NAME}}"
    step "Notarizing: $(basename "${target}")"
    [[ -f "${target}" ]] || die "File not found for notarization: ${target}"

    # --wait blocks until Apple returns a verdict (polls ~10 s intervals).
    local output
    output=$(xcrun notarytool submit "${target}" \
        --keychain-profile "${NOTARYTOOL_PROFILE}" \
        --wait 2>&1)

    echo "${output}"

    if echo "${output}" | grep -q "status: Accepted"; then
        ok "Notarization accepted by Apple."
    else
        # Extract submission UUID and fetch the detailed log for diagnosis.
        local uuid
        uuid=$(echo "${output}" | grep -Eo 'id: [0-9a-f-]{36}' | head -1 | awk '{print $2}')
        if [[ -n "${uuid}" ]]; then
            warn "Fetching notarization log for submission ${uuid}…"
            xcrun notarytool log "${uuid}" \
                --keychain-profile "${NOTARYTOOL_PROFILE}" \
                2>/dev/null || true
        fi
        die "Notarization failed or was rejected. Review the log above."
    fi
}

# ─────────────────────────────────────────────────────────────────────────────
#  STAPLE
# ─────────────────────────────────────────────────────────────────────────────

staple_target() {
    local target="${1:-${PACKAGE_DIR}/${DMG_NAME}}"
    step "Stapling notarization ticket: $(basename "${target}")"

    xcrun stapler staple "${target}" \
        || die "Stapling failed. Check that notarization completed successfully."

    ok "Ticket stapled."

    # Final Gatekeeper validation (requires the ticket to be embedded).
    info "Final Gatekeeper validation…"
    spctl --assess --type open --context context:primary-signature -v "${target}" \
        && ok "Gatekeeper validation passed — ready to distribute!" \
        || warn "Gatekeeper validation returned a warning (see output above)."
}

# ─────────────────────────────────────────────────────────────────────────────
#  STORE NOTARYTOOL CREDENTIALS  (run once per machine)
# ─────────────────────────────────────────────────────────────────────────────

store_credentials() {
    step "Storing notarytool credentials in Keychain"
    info "Profile name : ${NOTARYTOOL_PROFILE}"
    info "Apple ID     : ${APPLE_ID}"
    info "Team ID      : ${TEAM_ID}"
    echo ""
    xcrun notarytool store-credentials "${NOTARYTOOL_PROFILE}" \
        --apple-id  "${APPLE_ID}" \
        --team-id   "${TEAM_ID}" \
        --password  "${APP_PASSWORD}"
    ok "Credentials stored. Use --keychain-profile '${NOTARYTOOL_PROFILE}' in future calls."
}

# ─────────────────────────────────────────────────────────────────────────────
#  VERIFY EXISTING SIGNATURES
# ─────────────────────────────────────────────────────────────────────────────

verify() {
    step "Verifying existing signatures"
    for bundle in "${BUILD_DIR}/${MRV2_APP}" "${BUILD_DIR}/${HDR_APP:-__none__}"; do
        [[ -d "${bundle}" ]] || continue
        info "Checking: $(basename "${bundle}")"
        codesign --verify --deep --strict --verbose=2 "${bundle}" \
            && ok "  Signature OK" \
            || warn "  Signature check failed"
        spctl --assess --type exec --verbose "${bundle}" \
            && ok "  Gatekeeper OK" \
            || warn "  Gatekeeper check failed"
    done

    local dmg="${PACKAGE_DIR}/${DMG_NAME}"
    if [[ -f "${dmg}" ]]; then
        info "Checking DMG: $(basename "${dmg}")"
        codesign --verify --verbose=2 "${dmg}" \
            && ok "  DMG signature OK" \
            || warn "  DMG signature check failed"
    fi
}

# ─────────────────────────────────────────────────────────────────────────────
#  USAGE
# ─────────────────────────────────────────────────────────────────────────────

usage() {
cat <<EOF

${_C}mrv2 macOS Code Sign & Notarize Script${_N}
${_C}BSD-3-Clause — mrv2 Project Contributors${_N}

USAGE
  $(basename "$0") [OPTIONS] COMMAND

COMMANDS
  all           Full pipeline: sign-bundles → create-dmg → sign-dmg
                               → notarize → staple
  sign-bundles  Sign mrv2.app (and hdr.app) in-place
  create-dmg    Package signed .app bundles into a distributable DMG
  sign-dmg      Sign the DMG with the Developer ID certificate
  notarize      Submit the signed DMG to Apple Notary Service and wait
  staple        Staple the notarization ticket to the DMG
  verify        Verify existing signatures without changing anything
  store-creds   Store notarytool credentials in Keychain (run once)
  entitlements  Generate a default entitlements.plist and exit

OPTIONS
  -d, --dir DIR           Root Build output dir containing .app bundles and DMG
                          [default: \$BUILD_DIR or ./BUILD/CPackConfig]
  -i, --identity NAME     Developer ID Application certificate name
  -e, --entitlements PATH Path to entitlements.plist
                          [default: ./etc/macOS/mrv2.entitlements]
  -p, --profile NAME      notarytool Keychain profile name
                          [default: mrv2-notarytool]
      --no-hdr            Skip signing/packaging of hdr.app
  -h, --help              Show this help

ENVIRONMENT VARIABLES (override defaults without CLI flags)
  DEVELOPER_ID   APPLE_ID   TEAM_ID   APP_PASSWORD
  NOTARYTOOL_PROFILE   BUILD_DIR   MRV2_APP   HDR_APP
  DMG_NAME   ENTITLEMENTS

FIRST-TIME SETUP
  # 1. Store credentials once (interactive; asks for your app-specific password):
  $(basename "$0") store-creds

  # 2. Full pipeline:
  $(basename "$0") --build-dir BUILD/CPackConfig all

TYPICAL CI USAGE
  export DEVELOPER_ID="Developer ID Application: Gonzalo Garramuño (ABCDE12345)"
  export TEAM_ID="ABCDE12345"
  export NOTARYTOOL_PROFILE="mrv2-notarytool"
  $(basename "$0") all

NOTES
  • Requires Xcode 13+ for xcrun notarytool (the modern notarization tool).
    macOS 12 Monterey or later is recommended on the build machine.
  • Run 'store-creds' once per machine before using 'notarize' or 'all'.
    Credentials are stored securely in the login keychain.
  • If CPack already produced a DMG, skip 'create-dmg' and run:
      $(basename "$0") sign-bundles
      $(basename "$0") sign-dmg
      $(basename "$0") notarize
      $(basename "$0") staple
  • For arm64 / universal builds, no additional flags are needed — codesign
    handles fat binaries transparently.

EOF
}

# ─────────────────────────────────────────────────────────────────────────────
#  ARGUMENT PARSING
# ─────────────────────────────────────────────────────────────────────────────

COMMAND=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        -d|--root-dir)      ROOT_DIR="$2";           shift 2 ;;
        -vk)
	    ROOT_DIR="Darwin-vulkan-${ARCH}";
	    mrv2_NAME="vmrv2"
	    VK_ARG="-vk"
	    shift 1 ;;
        -i|--identity)      DEVELOPER_ID="$2";        shift 2 ;;
        -e|--entitlements)  ENTITLEMENTS="$2";        shift 2 ;;
        -p|--profile)       NOTARYTOOL_PROFILE="$2";  shift 2 ;;
        --no-hdr)           HDR_APP="";               shift   ;;
        -h|--help)          usage; exit 0 ;;
        all|sign-bundles|create-dmg|sign-dmg|notarize|staple|verify|store-creds|entitlements)
            COMMAND="$1"; shift ;;
        *)  echo -e "${_R}[ERR ]${_N}  Unknown argument: $1" >&2; usage; exit 1 ;;
    esac
done

if [[ -z "${COMMAND}" ]]; then
    usage
    exit 1
fi

export BUILD_DIR="${ROOT_DIR}/Release/"
export PACK_DIR="${BUILD_DIR}/mrv2/src/mrv2-build/_CPack_Packages/Darwin/DragNDrop/${mrv2_NAME}-v${mrv2_VERSION}-${KERNEL}-${ARCH}"
export PACKAGE_DIR="packages/${BUILD_DIR}"

# Name of the DMG produced by CPack (or the one this script creates).
DMG_NAME="${mrv2_NAME}-v${mrv2_VERSION}-${KERNEL}-${ARCH}.dmg"

# ─────────────────────────────────────────────────────────────────────────────
#  DISPATCH
# ─────────────────────────────────────────────────────────────────────────────

case "${COMMAND}" in

    entitlements)
        generate_entitlements
        ;;

    store-creds)
        store_credentials
        ;;

    sign-bundles)
        check_dependencies
        check_certificate
        generate_entitlements
        sign_all_bundles
        ;;

    create-dmg)
        check_dependencies
        #create_dmg
        ;;

    sign-dmg)
        check_dependencies
        check_certificate
        sign_dmg
        ;;

    notarize)
        check_dependencies
        notarize
        ;;

    staple)
        check_dependencies
        staple_target
        ;;

    verify)
        check_dependencies
        verify
        ;;

    all)
        check_dependencies
        check_certificate
        generate_entitlements
        sign_all_bundles
        #create_dmg
        sign_dmg
        notarize
        staple_target
        echo ""
        ok "────────────────────────────────────────────"
        ok " mrv2 is signed, notarized, and ready to ship!"
        ok "────────────────────────────────────────────"
        ;;

esac
