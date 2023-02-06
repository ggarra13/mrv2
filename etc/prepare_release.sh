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
GIT_EXECUTABLE=git


#
# Pull last changes
#
echo "Switch to main and pull changes"
${GIT_EXECUTABLE} switch main && ${GIT_EXECUTABLE} pull

echo "Now push any commited changes"
${GIT_EXECUTABLE} push

extract_version
echo "--------------------------------"
echo "  Will release v${mrv2_VERSION}"
echo "--------------------------------"



#
# Delete local tag if available
#
echo "Remove local tag v${mrv2_VERSION}"
${GIT_EXECUTABLE} tag -d v${mrv2_VERSION}

#
# Delete remote tag if available
#
echo "Remove remote tag v${mrv2_VERSION}"
${GIT_EXECUTABLE} push --delete origin v${mrv2_VERSION}



#
# Mark current repository with a new tag
#
echo "Create local tag v${mrv2_VERSION}"
${GIT_EXECUTABLE} tag v${mrv2_VERSION}

#
# Send new tag to repository
#
echo "Create remote tag v${mrv2_VERSION}"
${GIT_EXECUTABLE} push origin v${mrv2_VERSION}


echo "Cloning into an empty staging area to avoid .dockerignore bugs"
cd ..
mkdir -p release
cd release
${GIT_EXECUTABLE} clone ../mrv2

cd mrv2
