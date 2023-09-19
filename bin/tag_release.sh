#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

. ./etc/functions.sh

#
# This script tags a release both locally and in the remote repository.  It
# should be run before an actualy release.
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


add_local_tag()
{
    tag=$1
    export has_tag=`${GIT_EXECUTABLE} tag -l | grep "${tag}"`
    if [[ $has_tag != "" ]]; then
	#
	# Delete local tag if available
	#
	echo "Remove local tag '${tag}'"
	${GIT_EXECUTABLE} tag -d "${tag}"
    fi
    
    #
    # Mark current repository with a new tag
    #
    echo "Create local tag ${tag} in tlRender"
    ${GIT_EXECUTABLE} tag "${tag}"
}

#
# Prepare the git repository for release
#

#
# Pull last changes
#
echo "Switch to main and pull changes"
${GIT_EXECUTABLE} switch main && ${GIT_EXECUTABLE} pull

echo "Now push any commited changes"
${GIT_EXECUTABLE} push


export tag="v${mrv2_VERSION}"
echo "--------------------------------"
echo "  Will release ${tag}"
echo "--------------------------------"

add_local_tag $tag

cd tlRender

add_local_tag $tag

cd ..


input='y'
export has_tag=`${GIT_EXECUTABLE} ls-remote --tags origin | grep "${tag}"`
echo "has_tag?$has_tag"
if [[ $has_tag != "" ]]; then
    echo "-------------------------------------------------------"
    echo "  WARNING! Tag '${tag}' already in remote repository."
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
    ${GIT_EXECUTABLE} push --delete origin "${tag}"
else
    echo "Tag '${tag}' does not exist in remote"
    echo ""
    echo "Are you sure you want to continue? (y/n)"
    read input
    if [[ $input == n* || $input == N* ]]; then
	exit 1
    fi
fi

#
# Send new tag to repository
#
echo "Create remote tag ${tag}"
${GIT_EXECUTABLE} push origin "${tag}"
