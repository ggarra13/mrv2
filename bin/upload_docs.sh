#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.
# Define the directory where you want to start the search

. etc/functions.sh

extract_version

start_dir="docs/sphinx/"

# Use find command to recursively search for files
# Pipe the results to xargs, which will execute sed command on each file
find "$start_dir" -type f -exec sed -i "s/v[0-9]\.[0-9]\.[0-9]/v${mrv2_VERSION}/g" {} +

./runmeq.sh -t doc

rsync -avP --exclude '*~' -e ssh docs/www/* ggarra13@web.sourceforge.net:/home/project-web/mrv2/htdocs

rsync -avP --exclude '*~' -e ssh mrv2/docs/* ggarra13@web.sourceforge.net:/home/project-web/mrv2/htdocs/docs
