#!/usr/bin/env bash

release=$1

if [[ "$TAG" == "" ]]; then
    release=`git ls-remote --tags --refs 2> /dev/null | grep -E 'v[0-9]+\.[0-9]+\.[0-9]+$' | tail -n1 | cut -d/ -f3`
fi

project='mrv2'

API_KEY='568e57c2-5865-4a83-9ffc-1219d88be13d'


download_site="https://sourceforge.net/projects/${project}/files/${release}"

# Declare an associative array
declare -A platforms

platforms["windows"]='Windows-amd64.exe'
platforms["mac"]='Darwin-amd64.dmg'
platforms["linux"]='Linux-amd64.deb'

function change_default()
{
    platform=$1
    name=$2

    filename="${download_site}/${project}-${release}-${name}"
    
    echo "Changing ${filename}"
    
    curl -H "Accept: application/json" -X PUT -d "default=${platform}" -d "api_key=${API_KEY}" "${filename}"

}    

for platform in "${!platforms[@]}"; do
    change_default $platform ${platforms[$platform]}
done

