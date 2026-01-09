#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
# This script uploads the web pages components of mrv2 but not the
# online documentation.
#

NOARGS=1
. etc/build_dir.sh


latest_tag=$(git ls-remote --tags --sort="v:refname" https://github.com/ggarra13/mrv2.git | tail -n1 | sed 's#.*/##')

if [[ "$1" != "" ]]; then
    latest_tag=$1
fi

echo "UPDATING web pages to ${latest_tag}"

CMD='sed -i'
if [[ $KERNEL == *Darwin* ]]; then
    CMD='perl -pi -e'
fi

# Use find command to recursively search for files
find "docs/www/" -type f -name '*.html' -exec $CMD "s/v[0-9]\.[0-9]\.[0-9]/${latest_tag}/g" {} +


rsync -avP --exclude '*~' -e ssh docs/www/* ggarra13@web.sourceforge.net:/home/project-web/mrv2/htdocs
