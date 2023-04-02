#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/usr/bin/env bash

#
# This script takes a Python compiled on windows with PCbuild/build.bat and
# installs it following the standard layout set in the Windows' installers.
#


. etc/build_dir.sh

INSTALL_DIR=$BUILD_DIR/install/bin

mkdir -p $INSTALL_DIR/libs
mkdir -p $INSTALL_DIR/include
mkdir -p $INSTALL_DIR/Lib
mkdir -p $INSTALL_DIR/DLLs
mkdir -p $INSTALL_DIR/Scripts
mkdir -p $INSTALL_DIR/tcl
mkdir -p $INSTALL_DIR/Tools

PYTHON_ROOT=$BUILD_DIR/Python-prefix/src/Python
platform=$ARCH
if [[ "$ARCH" != "amd64" ]]; then
    platform=win32
fi
PYTHON_STAGE=$PYTHON_ROOT/PCbuild/${platform}

ls $PYTHON_STAGE/python*.exe
cp $PYTHON_STAGE/python*.exe $INSTALL_DIR/
cp $PYTHON_STAGE/python*.lib $INSTALL_DIR/libs
cp $PYTHON_STAGE/py*.dll $INSTALL_DIR/
cp $PYTHON_STAGE/lib*.dll $INSTALL_DIR/DLLs
cp $PYTHON_STAGE/sql*.dll $INSTALL_DIR/DLLs
cp $PYTHON_STAGE/tcl*.dll $INSTALL_DIR/DLLs
cp $PYTHON_STAGE/tk*.dll  $INSTALL_DIR/DLLs
cp $PYTHON_STAGE/*.pyd    $INSTALL_DIR/DLLs

cp -rf $PYTHON_ROOT/PC/py*.h  $INSTALL_DIR/include/
cp -rf $PYTHON_ROOT/Doc       $INSTALL_DIR/
cp -rf $PYTHON_ROOT/Include   $INSTALL_DIR/
cp -rf $PYTHON_ROOT/Lib       $INSTALL_DIR/
cp -rf $PYTHON_ROOT/Scripts   $INSTALL_DIR/
cp -rf $PYTHON_ROOT/Tools     $INSTALL_DIR/

cp -rf $PYTHON_ROOT/externals/tcltk-*/$platform/lib/* $INSTALL_DIR/tcl
