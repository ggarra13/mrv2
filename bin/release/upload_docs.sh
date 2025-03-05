#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.
# Define the directory where you want to start the search

. etc/build_dir.sh

extract_version

start_dir="docs/sphinx/"

CMD='sed -i'
if [[ $KERNEL == *Darwin* ]]; then
    CMD='perl -pi -e'
fi

# Use find command to recursively search for files
find "$start_dir" -type f -name '*.rst' -exec $CMD "s/v[0-9]\.[0-9]\.[0-9]/v${mrv2_VERSION}/g" {} +

./runmeq.sh -t doc

if [[ $KERNEL == *Msys* ]]; then
    pacman -Sy rsync --noconfirm
fi

rsync -avP --exclude '*~' -e ssh docs/www/* ggarra13@web.sourceforge.net:/home/project-web/mrv2/htdocs

rsync -avP --exclude '*~' -e ssh src/docs/* ggarra13@web.sourceforge.net:/home/project-web/mrv2/htdocs/docs
