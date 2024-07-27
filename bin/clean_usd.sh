#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
#
# Small build script for mrv2.  Run it from the root of the mrv2 dir, like:
#
# ./bin/runme_small.sh
#
# It builds all dependencies and will install
# the main executable on BUILD_DIR (by default
#                                   BUILD-OS-ARCH/BUILD_TYPE/install/bin).
#
# The small build does not have Python, USD, PDF, NDI or NETWORK support.
# It is intended for solo artists.
# 
#
# On Linux and macOS, it will also create a mrv2 or mrv2-dbg symbolic link
# in $HOME/bin if the directory exists.
#
# It will also log the compilation on $BUILD_DIR/compile.log
#


#
# Find out our build dir
#
. etc/build_dir.sh

export INSTALL=$BUILD_DIR/install

rm -rf $INSTALL/build
rm -rf $INSTALL/cmake/pxr*
rm -rf $INSTALL/include/MaterialX*
rm -rf $INSTALL/include/opensubdiv*
rm -rf $INSTALL/include/pxr*
rm -rf $INSTALL/include/serial*
rm -rf $INSTALL/include/tbb*
rm -rf $INSTALL/lib/libMaterial*
rm -rf $INSTALL/lib/libtbb*
rm -rf $INSTALL/lib/libusd*
rm -rf $INSTALL/lib/usd*
rm -rf $INSTALL/libosd*
rm -rf $INSTALL/libraries
rm -rf $INSTALL/plugin
rm -rf $INSTALL/pxrConfig*
rm -rf $INSTALL/resources
rm -rf $INSTALL/README.md
rm -rf $INSTALL/src
rm -rf $INSTALL/THIRD_PARTY

