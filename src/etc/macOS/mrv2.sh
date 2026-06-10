#!/bin/bash
# =============================================================================
# mrv2 / vmrv2 macOS Launcher
# Placed at: Contents/MacOS/mrv2 (or vmrv2)
# SPDX-License-Identifier: BSD-3-Clause
# =============================================================================

set -euo pipefail

# --- Resolve bundle root reliably ---
# This handles symlinks, different invocation methods, and macOS bundle quirks
SCRIPT_PATH="${BASH_SOURCE[0]}"
while [ -L "$SCRIPT_PATH" ]; do
    DIR="$(cd -P "$(dirname "$SCRIPT_PATH")" && pwd)"
    SCRIPT_PATH="$(readlink -f "$SCRIPT_PATH")"
    [[ "$SCRIPT_PATH" != /* ]] && SCRIPT_PATH="$DIR/$SCRIPT_PATH"
done

BUNDLE_DIR="$(cd -P "$(dirname "$SCRIPT_PATH")/.." && pwd)"
RESOURCES_DIR="$BUNDLE_DIR/Resources"
BIN_DIR="$RESOURCES_DIR/bin"
LIB_DIR="$RESOURCES_DIR/lib"

# --- Environment setup ---
export DYLD_LIBRARY_PATH="$LIB_DIR:${DYLD_LIBRARY_PATH:-}"
export DYLD_FALLBACK_LIBRARY_PATH="$LIB_DIR"

# Vulkan / MoltenVK setup
export VK_ICD_FILENAMES="$RESOURCES_DIR/etc/vulkan/icd.d/MoltenVK_icd.json${VK_ICD_FILENAMES:+:$VK_ICD_FILENAMES}"
export MVK_CONFIG_LOG_LEVEL=0

# Layer paths (Apple Silicon + Intel fallbacks)
export VK_LAYER_PATH="$RESOURCES_DIR/etc/vulkan/:${VK_LAYER_PATH:-}"
export VK_LAYER_PATH="$VK_LAYER_PATH:/opt/homebrew/share/vulkan/explicit_layers.d"
export VK_LAYER_PATH="$VK_LAYER_PATH:/usr/local/opt/vulkan-validationlayers/share/vulkan/explicit_layer.d"

# Optional: Add other env vars here (PYTHONHOME, OCIO, etc.)
#           or in $BIN_DIR/environment.sh

# --- Launch real binary ---
exec "$BIN_DIR/mrv2" "$@"
