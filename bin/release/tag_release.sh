#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set -e

. ./etc/functions.sh

#
# This script tags a release both locally and in the remote repository.  It
# should be run before an actual release.
#
# You must run it from the root of the mrv2 project.
#

#
# Extract the version from ./cmake/version.cmake
#
extract_version

#
# SOME DEFINES
#
export GIT_EXECUTABLE=git
export tag="v${mrv2_VERSION}"
export release_branch="release/${mrv2_VERSION}"

# ---------------------------------------------
# Sanity checks
# ---------------------------------------------
if ! ${GIT_EXECUTABLE} diff --quiet; then
    echo "ERROR: Working tree is dirty. Commit or stash changes first."
    exit 1
fi

# ---------------------------------------------
# Find previous release tag (if any)
# ---------------------------------------------
prev_tag=$(${GIT_EXECUTABLE} tag --sort=-version:refname | grep '^v' | head -n 1)

echo "--------------------------------"
echo " Preparing release ${tag}"
echo " Release branch: ${release_branch}"
echo " Previous tag: ${prev_tag:-<none>}"
echo "--------------------------------"

# ---------------------------------------------
# Create release branch from current HEAD
# ---------------------------------------------
if ${GIT_EXECUTABLE} show-ref --verify --quiet refs/heads/${release_branch}; then
    echo "ERROR: Release branch ${release_branch} already exists."
    exit 1
fi

${GIT_EXECUTABLE} checkout -b "${release_branch}"

# ---------------------------------------------
# Squash commits since previous release
# ---------------------------------------------
if [[ -n "${prev_tag}" ]]; then
    echo "Squashing commits since ${prev_tag}"
    ${GIT_EXECUTABLE} reset --soft "${prev_tag}"
else
    echo "No previous tag found — initial release"
fi

${GIT_EXECUTABLE} commit -m "Release ${mrv2_VERSION}"

# ---------------------------------------------
# Tag the release (local)
# ---------------------------------------------
if ${GIT_EXECUTABLE} rev-parse "${tag}" >/dev/null 2>&1; then
    echo "Removing existing local tag ${tag}"
    ${GIT_EXECUTABLE} tag -d "${tag}"
fi

${GIT_EXECUTABLE} tag -a "${tag}" -m "mrv2 ${mrv2_VERSION}"

# ---------------------------------------------
# Push release branch + tag
# ---------------------------------------------
echo "--------------------------------"
echo " Pushing ${release_branch} and ${tag}"
echo "--------------------------------"

${GIT_EXECUTABLE} push --force origin "${release_branch}"
${GIT_EXECUTABLE} push --force origin "${tag}"

echo "--------------------------------"
echo " Release ${tag} complete"
echo "--------------------------------"

