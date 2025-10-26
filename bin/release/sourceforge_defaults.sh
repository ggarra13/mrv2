#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

release=$1

if [[ "$release" == "" ]]; then
    release=`git ls-remote --tags --refs 2> /dev/null | grep -E 'v[0-9]+\.[0-9]+\.[0-9]+$' | tail -n1 | cut -d/ -f3`
fi

project='mrv2'

vmrv2='vmrv2'
mrv2='mrv2'

API_KEY='568e57c2-5865-4a83-9ffc-1219d88be13d'


download_site="https://sourceforge.net/projects/${project}/files/${release}"

# Use parallel arrays for platforms and filenames
platforms=("windows" "mac" "linux")
files=(
    "${vmrv2}-${release}-Windows-amd64.exe"
    "${mrv2}-${release}-Darwin-amd64.dmg"
    "${vmrv2}-${release}-Linux-amd64.deb"
)

change_default() {
    platform=$1
    name=$2

    filename="${download_site}/${name}"

    echo "Changing ${filename} with curl"
    which curl

    err=$(curl -s -H "Accept: application/json" -X PUT -d "default=${platform}" -d "api_key=${API_KEY}" "${filename}")
    echo "Returned status=$? $err"
    if [[ $? -ne 0 ]]; then
        echo "Returned status=$?"
        echo "$err"
        exit 1
    fi
    if [[ "$err" == *"code"* ]]; then
        echo "$err"
        exit 0
    fi
}

for i in "${!platforms[@]}"; do
    change_default "${platforms[$i]}" "${files[$i]}"
done
