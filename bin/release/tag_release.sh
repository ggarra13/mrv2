#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

. ./etc/functions.sh

#
# This script tags a release both locally and in the remote repository.  It
# should be run before an actual release.
#
# You must run it from the root of the mrv2 project.
#
#
# SOME DEFINES
#
export GIT_EXECUTABLE=git


${GIT_EXECUTABLE} switch peace


#
# Extract the version from ./cmake/version.cmake
#
extract_version


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
    ${GIT_EXECUTABLE} tag "${tag}"
}

add_remote_tag()
{
    input='y'
    export has_tag=`${GIT_EXECUTABLE} ls-remote --tags origin | grep "${tag}"`
    echo "has_tag? $has_tag"
    if [[ $has_tag != "" ]]; then
        echo "-------------------------------------------------------"
        echo "  WARNING! Tag '${tag}' already in remote repository."
        echo ""
        echo "Are you sure you want to continue? (y/n)"
        read input
        if [[ $input != y* && $input != Y* ]]; then
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
        if [[ $input != y* && $input != Y* ]]; then
	    exit 1
        fi
    fi

    #
    # Send new tag to repository
    #
    echo "Create remote tag ${tag}"
    ${GIT_EXECUTABLE} push origin "${tag}"
}

#
# 
#
${GIT_EXECUTABLE} switch main
${GIT_EXECUTABLE} merge peace --squash
${GIT_EXECUTABLE} commit -m "Release ${tag}"
${GIT_EXECUTABLE} push --force origin main

#
# Prepare the git repository for release
#

#
# Pull last changes
#
export tag="v${mrv2_VERSION}"

echo "--------------------------------"
echo "  Will release local ${tag} in mrv2"
echo "--------------------------------"

add_local_tag $tag

echo "---------------------------------------"
echo "  Will release remote ${tag} in mrv2"
echo "---------------------------------------"

add_remote_tag $tag


${GIT_EXECUTABLE} switch peace
${GIT_EXECUTABLE} merge main
