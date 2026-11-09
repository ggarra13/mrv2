#!/usr/bin/env bash

#set -e

. etc/functions.sh

if [[ "$1" == "" ]]; then
    extract_version
else
    export mrv2_VERSION=$1
fi

echo
echo "RELEASING chocolatey for ${mrv2_VERSION}"
echo

START_DIR=$PWD

cd src/etc/windows/chocolatey/mrv2

for mrv2_NAME in mrv2 vmrv2; do
    build_dir=build-${mrv2_NAME}
    cmake -S . -B ${build_dir} -D MRV2_NAME=${mrv2_NAME} -D MRV2_VERSION=${mrv2_VERSION}
    cmake --build ${build_dir}  --target choco_pack

    cd ${build_dir}/package
    choco push ${mrv2_NAME}.${mrv2_VERSION}.nupkg --source https://push.chocolatey.org/
    cd -
    
    rm -rf ${build_dir}
done

cd $START_DIR
