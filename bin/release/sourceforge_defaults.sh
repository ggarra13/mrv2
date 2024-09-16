#!/usr/bin/env bash

release=$1

if [[ "$release" == "" ]]; then
    release=`git ls-remote --tags --refs 2> /dev/null | grep -E 'v[0-9]+\.[0-9]+\.[0-9]+$' | tail -n1 | cut -d/ -f3`
fi

project='mrv2'

API_KEY='568e57c2-5865-4a83-9ffc-1219d88be13d'


download_site="https://sourceforge.net/projects/${project}/files/${release}"

# Declare an associative array
declare -A platforms

platforms["windows"]="${project}-${release}-Windows-amd64.exe"
platforms["mac"]="${project}-${release}-Darwin-amd64.dmg"
platforms["linux"]="${project}-${release}-Linux-amd64.deb"

function change_default()
{
    platform=$1
    name=$2

    filename="${download_site}/${name}"
    
    echo "Changing ${filename}"
    
    err=`curl -s -H "Accept: application/json" -X PUT -d "default=${platform}" -d "api_key=${API_KEY}" "${filename}"`
    if [[ $? != 0 ]]; then
	echo "Returned status=$?"
	echo $err
	exit 1
    fi
    if [[ "$err" == "*code*" ]]; then
	echo $err
	exit 1
    fi
}    

for platform in "${!platforms[@]}"; do
    change_default $platform ${platforms[$platform]}
done

