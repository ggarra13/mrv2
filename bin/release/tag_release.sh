#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2 release script

set -euo pipefail

DEV_BRANCH="peace"
MAIN_BRANCH="main"

. ./etc/functions.sh
extract_version

TAG="v${mrv2_VERSION}"

# ---------------------------------------------
# Safety checks
# ---------------------------------------------
current_branch=$(git symbolic-ref --short HEAD)

if [[ "${current_branch}" != "${DEV_BRANCH}" ]]; then
    echo "ERROR: You must run this script from '${DEV_BRANCH}'."
    exit 1
fi

if ! git diff --quiet; then
    echo "ERROR: Working tree is dirty."
    exit 1
fi

# Ensure main has NOT been merged into
if git merge-base --is-ancestor "${DEV_BRANCH}" "${MAIN_BRANCH}" 2>/dev/null; then
    echo "ERROR: ${MAIN_BRANCH} contains ${DEV_BRANCH} history."
    echo "This breaks the release model. Aborting."
    exit 1
fi

# ---------------------------------------------
# Determine previous release tag
# ---------------------------------------------
PREV_TAG=$(git tag --sort=-version:refname | grep '^v' | head -n 1 || true)

echo "--------------------------------"
echo " Releasing ${TAG}"
echo " From branch ${DEV_BRANCH}"
echo " Previous tag: ${PREV_TAG:-<none>}"
echo "--------------------------------"

# ---------------------------------------------
# Create snapshot on main
# ---------------------------------------------
git checkout "${MAIN_BRANCH}" 2>/dev/null || git checkout -b "${MAIN_BRANCH}"

git reset --soft "${PREV_TAG:-HEAD}"
git commit -m "Release ${mrv2_VERSION}"

git tag -a "${TAG}" -m "mrv2 ${mrv2_VERSION}"

# ---------------------------------------------
# Push snapshot + tag
# ---------------------------------------------
git push -f origin "${MAIN_BRANCH}"
git push -f origin "${TAG}"

# ---------------------------------------------
# Return to dev branch
# ---------------------------------------------
git checkout "${DEV_BRANCH}"

echo "--------------------------------"
echo " Release ${TAG} completed"
echo "--------------------------------"
