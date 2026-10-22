#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RESOURCES_DIR="$(cd -P "$SCRIPT_DIR/../Resources" && pwd)"
BIN="$RESOURCES_DIR/bin/mrv2"

if [[ ! -x "$BIN" ]]; then
    echo "mrv2: missing executable: $BIN" >&2
    exit 1
fi

# Only bundled Vulkan configuration. No inherited or system-wide paths.
export VK_ICD_FILENAMES="$RESOURCES_DIR/etc/vulkan/icd.d/MoltenVK_icd.json"
export VK_LAYER_PATH="$RESOURCES_DIR/etc/vulkan"
export MVK_CONFIG_LOG_LEVEL=0

exec "$BIN" "$@"
