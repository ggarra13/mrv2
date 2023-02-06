#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


if [[ $KERNEL != *Msys* ]]; then
    chmod a+x $PWD/$BUILD_DIR/install/bin/mrv2.sh
    if [[ -d ~/bin ]]; then
	if [[ $CMAKE_BUILD_TYPE == Debug ]]; then
	    rm -f ~/bin/mrv2-dbg
	    ln -s $PWD/$BUILD_DIR/install/bin/mrv2.sh ~/bin/mrv2-dbg
	else
	    rm -f ~/bin/mrv2
	    ln -s $PWD/$BUILD_DIR/install/bin/mrv2.sh ~/bin/mrv2
	fi
    fi
fi

if [[ $CMAKE_BUILD_TYPE == Release ]]; then
    #
    # Here we move the installers to the packages location.
    # Note that we don't move the Linux packages as those are built
    # with Docker to make them compatible across multiple Linux versions-
    #
    if [[ $KERNEL == *Msys* ]]; then
	send_to_packages mrv2-v${mrv2_VERSION}-Windows-${ARCH}.exe
	send_to_packages mrv2-v${mrv2_VERSION}-Windows-${ARCH}.zip
    elif [[ $KERNDER == *Darwin* ]]; then
	send_to_packages mrv2-v${mrv2_VERSION}-Darwin-${ARCH}.dmg
    fi
fi
