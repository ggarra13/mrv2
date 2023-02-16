#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

. ./etc/functions.sh


#
# Prepare the git repository for release
#

#
# SOME DEFINES
#
export GIT_EXECUTABLE=git


#
# Pull last changes
#
echo "Switch to main and pull changes"
${GIT_EXECUTABLE} switch main && ${GIT_EXECUTABLE} pull

echo "Now push any commited changes"
${GIT_EXECUTABLE} push

extract_version

export tag="v${mrv2_VERSION}"
echo "--------------------------------"
echo "  Will release ${tag}"
echo "--------------------------------"


export has_tag=`${GIT_EXECUTABLE} tag -l | grep ${tag}`
if [[ $has_tag != "" ]]; then
    #
    # Delete local tag if available
    #
    echo "Remove local tag ${tag}"
    ${GIT_EXECUTABLE} tag -d ${tag}
fi

#
# Mark current repository with a new tag
#
echo "Create local tag ${tag}"
${GIT_EXECUTABLE} tag ${tag}

input='y'
export has_tag=`${GIT_EXECUTABLE} ls-remote --tags origin | grep ${tag}`
echo "has_tag?$has_tag"
if [[ $has_tag != "" ]]; then
    echo "-------------------------------------------------------"
    echo "  WARNING! Tag ${tag} already in remote repository."
    echo ""
    echo "Are you sure you want to continue? (y/n)"
    read input
    if [[ $input == n* || $input == N* ]]; then
	exit 1
    fi

    #
    # Delete remote tag if available
    #
    echo "Remove remote tag ${tag}"
    ${GIT_EXECUTABLE} push --delete origin ${tag}
else
    echo "Tag ${tag} does not exist in remote"
fi

#
# Send new tag to repository
#
echo "Create remote tag ${tag}"
${GIT_EXECUTABLE} push origin ${tag}
