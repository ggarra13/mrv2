#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set -e

. ./etc/functions.sh

extract_version

# DEFINES
export GIT_EXECUTABLE=git
export tag="v${mrv2_VERSION}"
export source_branch="peace"
export main_branch="main"

# ---------------------------------------------
# 1. Sanity checks
# ---------------------------------------------
if ! ${GIT_EXECUTABLE} diff --quiet; then
    echo "ERROR: Working tree is dirty. Commit or stash changes first."
    exit 1
fi

# Ensure we have the latest from the server
${GIT_EXECUTABLE} fetch origin

# ---------------------------------------------
# 2. Squash onto Main
# ---------------------------------------------
echo "Merging and squashing ${source_branch} onto ${main_branch}..."

${GIT_EXECUTABLE} checkout "${main_branch}"
${GIT_EXECUTABLE} pull origin "${main_branch}"

# Stage all changes from peace as a single set of changes
${GIT_EXECUTABLE} merge --squash "${source_branch}"

# Create the single "Squashed" commit
${GIT_EXECUTABLE} commit -m "Release ${mrv2_VERSION} (Squashed merge from ${source_branch})"

# ---------------------------------------------
# 3. Tagging
# ---------------------------------------------
if ${GIT_EXECUTABLE} rev-parse "${tag}" >/dev/null 2>&1; then
    echo "Removing existing local tag ${tag}"
    ${GIT_EXECUTABLE} tag -d "${tag}"
fi

${GIT_EXECUTABLE} tag -a "${tag}" -m "mrv2 ${mrv2_VERSION}"

# ---------------------------------------------
# 4. Push Main and Tag
# ---------------------------------------------
echo "Pushing ${main_branch} and ${tag} to origin..."
${GIT_EXECUTABLE} push origin "${main_branch}" --force-with-lease
${GIT_EXECUTABLE} push origin "${tag}" --force

# ---------------------------------------------
# 5. Synchronize "peace" branch (The Reset)
# ---------------------------------------------
echo "Synchronizing ${source_branch} with ${main_branch}..."

${GIT_EXECUTABLE} checkout "${source_branch}"

# This makes 'peace' identical to 'main', effectively "clearing" 
# the history that was just squashed.
${GIT_EXECUTABLE} reset --hard "${main_branch}"

# Force push the reset to the remote so others see the synchronized state
${GIT_EXECUTABLE} push --force origin "${source_branch}"

echo "--------------------------------"
echo " Release ${tag} complete."
echo " ${main_branch} is updated and ${source_branch} is synchronized."
echo "--------------------------------"
