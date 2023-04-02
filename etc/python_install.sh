#!/usr/bin/env bash

. build_dir.sh

INSTALL_DIR=$BUILD_DIR/install/bin

mkdir -p $INSTALL_DIR/Lib
mkdir -p $INSTALL_DIR/DLLs
mkdir -p $INSTALL_DIR/Scripts

PYTHON_ROOT=$BUILD_DIR/Python-prefix/src/Python
PYTHON_STAGE=$PYTHON_ROOT/PCbuild/${ARCH}

cp $PYTHON_STAGE/python*.dll $INSTALL_DIR
cp $PYTHON_STAGE/lib*.dll $INSTALL_DIR/DLLs
cp $PYTHON_STAGE/sql*.dll $INSTALL_DIR/DLLs
cp $PYTHON_STAGE/tcl*.dll $INSTALL_DIR/DLLs
cp $PYTHON_STAGE/tk*.dll  $INSTALL_DIR/DLLs
cp $PYTHON_STAGE/*.pyd    $INSTALL_DIR/DLLs

cp $PYTHON_ROOT/Lib       $INSTALL_DIR/Lib
cp $PYTHON_ROOT/Scripts   $INSTALL_DIR/Scripts
