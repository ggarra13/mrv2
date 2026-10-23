#!/usr/bin/env bash
#
# Unregister every Launch Services entry whose path matches "mrv2"
# (this also matches vmrv2), optionally re-registering one good install.
#
# Usage:
#   ./lsr_clean_mrv2.sh                          # dry run: only lists matches
#   ./lsr_clean_mrv2.sh --apply                  # unregister all matches
#   ./lsr_clean_mrv2.sh --apply --keep /Applications/mrv2.app
#                                                # unregister all, then
#                                                # re-register the kept one
#   ./lsr_clean_mrv2.sh --pattern 'vmrv2'        # use a different match pattern
#
set -u

LSR=/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister

APPLY=0
KEEP=""
PATTERN="mrv2"

while [ $# -gt 0 ]; do
    case "$1" in
        --apply)   APPLY=1 ;;
        --keep)    shift; KEEP="${1:-}" ;;
        --pattern) shift; PATTERN="${1:-mrv2}" ;;
        -h|--help)
            sed -n '2,15p' "$0"; exit 0 ;;
        *) echo "Unknown option: $1" >&2; exit 1 ;;
    esac
    shift
done

if [ ! -x "$LSR" ]; then
    echo "lsregister not found at $LSR" >&2
    exit 1
fi

# Dump lines look like:  path:    /Applications/mrv2.app (0x1234)
# Strip the "path:" prefix and the trailing " (0x...)" id, keep spaces in paths,
# and de-duplicate.
paths=()
while IFS= read -r line; do
    paths+=("$line")
done < <(
    "$LSR" -dump \
    | grep -E "^[[:space:]]*path:.*${PATTERN}" \
    | sed -E 's/^[[:space:]]*path:[[:space:]]*//; s/[[:space:]]+\(0x[0-9a-fA-F]+\)[[:space:]]*$//' \
    | sort -u
)

if [ ${#paths[@]} -eq 0 ]; then
    echo "No Launch Services entries match '${PATTERN}'."
    exit 0
fi

echo "Found ${#paths[@]} registered path(s) matching '${PATTERN}':"
for p in "${paths[@]}"; do
    if [ -e "$p" ]; then
        echo "  [exists ] $p"
    else
        echo "  [MISSING] $p"
    fi
done

if [ "$APPLY" -ne 1 ]; then
    echo
    echo "Dry run only. Re-run with --apply to unregister these."
    exit 0
fi

echo
for p in "${paths[@]}"; do
    echo "Unregistering: $p"
    "$LSR" -u "$p"
done

if [ -n "$KEEP" ]; then
    if [ -d "$KEEP" ]; then
        echo
        echo "Re-registering: $KEEP"
        "$LSR" -f "$KEEP"
    else
        echo "Warning: --keep path does not exist: $KEEP" >&2
    fi
fi

$LSR -gc

echo
echo "Done. Verify with:"
echo "  $LSR -dump | grep -E 'path:.*${PATTERN}'"
