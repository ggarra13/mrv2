#!/usr/bin/env bash

set -e

. etc/functions.sh

if [[ "$1" == "" ]]; then
    extract_version
else
    export mrv2_VERSION=$1
fi

echo "RELEASING CHOCOLATEY PACKAGE ${mrv2_VERSION}"

START_DIR=$PWD

BUILD_DIR=$PWD/src/etc/windows/chocolatey/mrv2
cd $BUILD_DIR

for mrv2_NAME in mrv2 vmrv2; do
    build_dir=$BUILD_DIR/build-${mrv2_NAME}
    echo "  BUILDING chocolatey package for ${mrv2_NAME} in ${build_dir}"
    
    cmake -S . -B ${build_dir} -D MRV2_NAME=${mrv2_NAME} -D MRV2_VERSION=${mrv2_VERSION}
    cmake --build ${build_dir}  --target choco_pack

    echo "   PUSHING chocolatey package for ${mrv2_NAME} in ${build_dir}"
    cd ${build_dir}/package
    choco push ${mrv2_NAME}.${mrv2_VERSION}.nupkg --source https://push.chocolatey.org/
    cd -
    
    rm -rf ${build_dir}
done

cd $START_DIR
exit 0
