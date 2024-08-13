#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
# Cleans up USD directories for an upgrade of OpenUSD.
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
rm -rf $INSTALL/lib/boost*
rm -rf $INSTALL/libosd*
rm -rf $INSTALL/libraries
rm -rf $INSTALL/plugin
rm -rf $INSTALL/pxrConfig*
rm -rf $INSTALL/resources
rm -rf $INSTALL/README.md
rm -rf $INSTALL/src
rm -rf $INSTALL/tlRender/etc/SuperBuild/USD
rm -rf $INSTALL/THIRD_PARTY

