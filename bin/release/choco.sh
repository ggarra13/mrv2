#!/usr/bin/env bash

set -e

. etc/functions.sh

if [[ "$1" == "" ]]; then
    extract_version
else
    export mrv2_VERSION=$1
fi

echo
echo "RELEASING ${mrv2_VERSION}"
echo

START_DIR=$PWD

cd src/etc/windows/chocolatey/mrv2

cmake -S . -B build-mrv2  -DMRV2_NAME=mrv2  -DMRV2_VERSION=${mrv2_VERSION} -DMRV2_ARCH=amd64
cmake --build build-mrv2  --target choco_pack

cd build-mrv2/package
choco push mrv2.${mrv2_VERSION}.nupkg --source https://push.chocolatey.org/
cd -
rm -rf build-mrv2

cmake -S . -B build-vmrv2 -DMRV2_NAME=vmrv2 -DMRV2_VERSION=${mrv2_VERSION} -DMRV2_ARCH=amd64
cmake --build build-vmrv2 --target choco_pack

cd build-vmrv2/package
choco push vmrv2.${mrv2_VERSION}.nupkg --source https://push.chocolatey.org/
cd -

rm -rf build-vmrv2

cd $START_DIR
